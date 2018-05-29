#include "convert.h"

namespace convert {
	/** FROM WIRE **/

	uid_t from_wire(tdcr::network::Uid from) {
		return uid_t {
			from.part1(), from.part2()
		};
	}

	void from_wire(tdcr::crypto::Mac128 from, sgx_cmac_128bit_tag_t& to) {
		*(uint64_t*)&to[0] = from.part1();
		*(uint64_t*)&to[8] = from.part2();
	}

	command_tag_t from_wire(tdcr::raft::CommandTag from) {
		return command_tag_t { from_wire(from.uid()), (command_type_t)from.type() };
	};

	entry_t from_wire(tdcr::raft::Entry from) {
		return entry_t { from.index(), from.term(), from_wire(from.event()), from_wire(from.source()), from_wire(from.tag()) };
	}

	dcr_event from_wire(tdcr::dcr::Event from) {
		return dcr_event {
			from_wire(from.uid()),
			from.executed(),
			from.excluded(),
			from.pending()
		};
	}

	dcr_workflow from_wire(tdcr::dcr::Workflow from) {
		dcr_workflow to;
		to.name = from.name();

		// events
		for each (tdcr::dcr::Event wire_event in from.events()) {
			dcr_event event = from_wire(wire_event);
			to.event_store[event.id] = event;

			// excludes
			for each (tdcr::network::Uid wire_target in wire_event.excluderelations())
				to.excludes_to[event.id].push_back(from_wire(wire_target));

			// includes
			for each (tdcr::network::Uid wire_target in wire_event.includerelations())
				to.includes_to[event.id].push_back(from_wire(wire_target));

			// responses
			for each (tdcr::network::Uid wire_target in wire_event.responserelations())
				to.responses_to[event.id].push_back(from_wire(wire_target));

			// conditions
			for each (tdcr::network::Uid wire_target in wire_event.conditionrelations()) {
				uid_t target = from_wire(wire_target);
				to.conditions_to[event.id].push_back(target);
				to.conditions_from[target].push_back(event.id);
			}

			// conditions
			for each (tdcr::network::Uid wire_target in wire_event.milestonerelations()) {
				uid_t target = from_wire(wire_target);
				to.milestones_to[event.id].push_back(target);
				to.milestones_from[target].push_back(event.id);
			}
		}

		return to;
	}

	/** TO WIRE **/

	tdcr::network::Uid to_wire(uid_t from) {
		tdcr::network::Uid to;
		to.set_part1(from.id1);
		to.set_part2(from.id2);
		return to;
	}

	tdcr::crypto::Mac128 to_wire(sgx_cmac_128bit_tag_t from) {
		tdcr::crypto::Mac128 to;
		to.set_part1(*(uint64_t*)&from[0]);
		to.set_part2(*(uint64_t*)&from[8]);
		return to;
	}

	tdcr::raft::CommandTag to_wire(command_tag_t from) {
		tdcr::raft::CommandTag to;
		to.set_allocated_uid(new tdcr::network::Uid(to_wire(from.uid)));
		to.set_type((tdcr::raft::CommandType)from.type);
		return to;
	}

	tdcr::raft::Entry to_wire(entry_t from) {
		tdcr::raft::Entry to;
		to.set_index(from.index);
		to.set_term(from.term);
		to.set_allocated_event(new tdcr::network::Uid(to_wire(from.event)));
		to.set_allocated_source(new tdcr::network::Uid(to_wire(from.source)));
		to.set_allocated_tag(new tdcr::raft::CommandTag(to_wire(from.tag)));
		return to;
	}

	/** MSG UNPACKING **/
	/* COMMAND */

	void unpack(tdcr::network::Container from, command_req_t& to) {
		tdcr::raft::CommandRequest payload;
		unpack_container(from, payload, to);

		to.tag = from_wire(payload.tag());
		to.event = from_wire(payload.event());
	}

	void unpack(tdcr::network::Container from, command_rsp_t& to) {
		tdcr::raft::CommandResponse payload;
		unpack_container(from, payload, to);

		to.tag = from_wire(payload.tag());
		to.success = payload.success();
		to.leader = from_wire(payload.leader());
	}

	/* APPEND */

	void unpack(tdcr::network::Container from, append_req_t& to) {
		tdcr::raft::AppendRequest payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.prev_term = payload.prev_term();
		to.prev_index = payload.prev_index();
		to.commit_index = payload.commit_index();

		to.entries_n = payload.entries_size();
		to.entries = new entry_t[to.entries_n];
		for (uint32_t i = 0; i < to.entries_n; i++)
			to.entries[i] = from_wire(payload.entries(i));
	}

	void unpack(tdcr::network::Container from, append_rsp_t& to) {
		tdcr::raft::AppendResponse payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.prev_term = payload.prev_term();
		to.prev_index = payload.prev_index();
		to.success = payload.success();
		to.last_index = payload.last_index();
	}

	/* POLL */

	void unpack(tdcr::network::Container from, poll_req_t& to) {
		tdcr::raft::PollRequest payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.last_term = payload.last_term();
		to.last_index = payload.last_index();
	}

	void unpack(tdcr::network::Container from, poll_rsp_t& to) {
		tdcr::raft::PollResponse payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.success = payload.success();
	}

	/* ELECTION */

	void unpack(tdcr::network::Container from, election_req_t& to) {
		tdcr::raft::ElectionRequest payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.last_term = payload.last_term();
		to.last_index = payload.last_index();
	}

	void unpack(tdcr::network::Container from, election_rsp_t& to) {
		tdcr::raft::ElectionResponse payload;
		unpack_container(from, payload, to);

		to.term = payload.term();
		to.success = payload.success();
	}

	/* LOG */

	void unpack(tdcr::network::Container from, log_req_t& to) {
		tdcr::raft::LogRequest payload;
		unpack_container(from, payload, to);
	}

	void unpack(tdcr::network::Container from, log_rsp_t& to) {
		tdcr::raft::LogResponse payload;
		unpack_container(from, payload, to);

		to.success = payload.success();
		to.leader = from_wire(payload.leader());

		to.entries_n = payload.entries_size();
		to.entries = new entry_t[to.entries_n];
		for (uint32_t i = 0; i < to.entries_n; i++)
			to.entries[i] = from_wire(payload.entries(i));
	}

	/** MSG PACKING **/
	/* COMMAND */

	tdcr::network::Container pack(command_req_t from) {
		tdcr::raft::CommandRequest payload;
		payload.set_allocated_tag(new tdcr::raft::CommandTag(to_wire(from.tag)));
		payload.set_allocated_event(new tdcr::network::Uid(to_wire(from.event)));
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_COMMAND_REQUEST);
	}

	tdcr::network::Container pack(command_rsp_t from) {
		tdcr::raft::CommandResponse payload;
		payload.set_allocated_tag(new tdcr::raft::CommandTag(to_wire(from.tag)));
		payload.set_success(from.success);
		payload.set_allocated_leader(new tdcr::network::Uid(to_wire(from.leader)));
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_COMMAND_RESPONSE);
	}

	/* APPEND */

	tdcr::network::Container pack(append_req_t from) {
		tdcr::raft::AppendRequest payload;
		payload.set_term(from.term);
		payload.set_prev_term(from.prev_term);
		payload.set_prev_index(from.prev_index);
		payload.set_commit_index(from.commit_index);
		for (uint32_t i = 0; i < from.entries_n; i++)
			*payload.add_entries() = to_wire(from.entries[i]);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_APPEND_REQUEST);
	}

	tdcr::network::Container pack(append_rsp_t from) {
		tdcr::raft::AppendResponse payload;
		payload.set_term(from.term);
		payload.set_prev_term(from.prev_term);
		payload.set_prev_index(from.prev_index);
		payload.set_success(from.success);
		payload.set_last_index(from.last_index);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_APPEND_RESPONSE);
	}

	/* POLL */

	tdcr::network::Container pack(poll_req_t from) {
		tdcr::raft::PollRequest payload;
		payload.set_term(from.term);
		payload.set_last_term(from.last_term);
		payload.set_last_index(from.last_index);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_POLL_REQUEST);
	}

	tdcr::network::Container pack(poll_rsp_t from) {
		tdcr::raft::PollResponse payload;
		payload.set_term(from.term);
		payload.set_success(from.success);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_POLL_RESPONSE);
	}

	/* ELECTION */

	tdcr::network::Container pack(election_req_t from) {
		tdcr::raft::ElectionRequest payload;
		payload.set_term(from.term);
		payload.set_last_term(from.last_term);
		payload.set_last_index(from.last_index);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_ELECTION_REQUEST);
	}

	tdcr::network::Container pack(election_rsp_t from) {
		tdcr::raft::ElectionResponse payload;
		payload.set_term(from.term);
		payload.set_success(from.success);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_ELECTION_RESPONSE);
	}

	/* LOG */

	tdcr::network::Container pack(log_req_t from) {
		tdcr::raft::LogRequest payload;
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_LOG_REQUEST);
	}

	tdcr::network::Container pack(log_rsp_t from) {
		tdcr::raft::LogResponse payload;
		payload.set_success(from.success);
		payload.set_allocated_leader(new tdcr::network::Uid(to_wire(from.leader)));
		for (uint32_t i = 0; i < from.entries_n; i++)
			*payload.add_entries() = to_wire(from.entries[i]);
		return pack_container(from, payload, tdcr::network::Container_PayloadType::Container_PayloadType_LOG_RESPONSE);
	}
}
