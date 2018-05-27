#pragma once
#include "msg.h"
#include "protos\raft.pb.h"

namespace convert {
	/** FROM WIRE **/

	uid_t from_wire(tdcr::network::Uid from);
	void from_wire(tdcr::crypto::Mac128 from, sgx_cmac_128bit_tag_t& to);
	command_tag_t from_wire(tdcr::raft::CommandTag from);
	entry_t from_wire(tdcr::raft::Entry from);

	/** TO WIRE **/

	tdcr::network::Uid to_wire(uid_t from);
	tdcr::crypto::Mac128 to_wire(sgx_cmac_128bit_tag_t from);
	tdcr::raft::CommandTag to_wire(command_tag_t from);
	tdcr::raft::Entry to_wire(entry_t from);

	/** MSG UNPACKING **/

	template<typename T, typename P>
	void unpack_container(tdcr::network::Container from, P& payload, T& to) {
		payload.ParsePartialFromString(from.payload());
		to.target = from_wire(from.target());
		to.source = from_wire(from.source());
		from_wire(from.mac(), to.mac);
	}

	/* SPECIALIZATION */

	void unpack(tdcr::network::Container from, command_req_t& to);
	void unpack(tdcr::network::Container from, command_rsp_t& to);

	void unpack(tdcr::network::Container from, append_req_t& to);
	void unpack(tdcr::network::Container from, append_rsp_t& to);

	void unpack(tdcr::network::Container from, poll_req_t& to);
	void unpack(tdcr::network::Container from, poll_rsp_t& to);

	void unpack(tdcr::network::Container from, election_req_t& to);
	void unpack(tdcr::network::Container from, election_rsp_t& to);

	void unpack(tdcr::network::Container from, log_req_t& to);
	void unpack(tdcr::network::Container from, log_rsp_t& to);

	/** MSG PACKING **/

	template<typename T, typename P>
	tdcr::network::Container pack_container(T from, P payload, tdcr::network::Container::PayloadType type) {
		tdcr::network::Container to;
		to.set_allocated_target(new tdcr::network::Uid(to_wire(from.target)));
		to.set_allocated_source(new tdcr::network::Uid(to_wire(from.source)));
		to.set_type(type);
		to.set_allocated_payload(new std::string(payload.SerializePartialAsString()));
		to.set_allocated_mac(new tdcr::crypto::Mac128(to_wire(from.mac)));
		return to;
	}

	/* SPECIALIZATION */

	tdcr::network::Container pack(command_req_t from);
	tdcr::network::Container pack(command_rsp_t from);

	tdcr::network::Container pack(append_req_t from);
	tdcr::network::Container pack(append_rsp_t from);

	tdcr::network::Container pack(poll_req_t from);
	tdcr::network::Container pack(poll_rsp_t from);

	tdcr::network::Container pack(election_req_t from);
	tdcr::network::Container pack(election_rsp_t from);

	tdcr::network::Container pack(log_req_t from);
	tdcr::network::Container pack(log_rsp_t from);
}
