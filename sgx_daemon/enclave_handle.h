#pragma once
#include <tchar.h>
#include <map>
#include <chrono>
#include "sgx_urts.h"
#include "enclave_u.h"
#include "dcr.h"

#define ENCLAVE_FILE _T("enclave.signed.dll")

class enclave_handle {
public:
	sgx_enclave_id_t eid;

	sgx_status_t init_enclave() {
		sgx_launch_token_t token;
		SecureZeroMemory(token, sizeof(token));
		int updated = 0;
		return sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
	}

  sgx_status_t destroy_enclave() {
    return sgx_destroy_enclave(eid);
  }

	sgx_status_t e_recv_command_req(command_req_t req) {
		return recv_command_req(eid, req);
	}

	sgx_status_t e_recv_command_rsp(command_rsp_t rsp) {
		return recv_command_rsp(eid, rsp);
	}

	sgx_status_t e_recv_append_req(append_req_t req) {
		return recv_append_req(eid, req);
	}

	sgx_status_t e_recv_append_rsp(append_rsp_t rsp) {
		return recv_append_rsp(eid, rsp);
	}

	sgx_status_t e_recv_poll_req(poll_req_t req) {
		return recv_poll_req(eid, req);
	}

	sgx_status_t e_recv_poll_rsp(poll_rsp_t rsp) {
		return recv_poll_rsp(eid, rsp);
	}

	sgx_status_t e_recv_election_req(election_req_t req) {
		return recv_election_req(eid, req);
	}

	sgx_status_t e_recv_election_rsp(election_rsp_t rsp) {
		return recv_election_rsp(eid, rsp);
	}

	sgx_status_t e_recv_log_req(log_req_t req) {
		return recv_log_req(eid, req);
	}

	sgx_status_t e_recv_log_rsp(log_rsp_t rsp) {
		return recv_log_rsp(eid, rsp);
	}

  sgx_status_t e_set_time() {
    uint64_t ticks = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return set_time(eid, ticks);
  }

  void flatten_map(std::map<uid_t, uid_t, cmp_uids> in_map, uid_t* peers_list, uid_t* events_list) {
    std::vector<uid_t> peers;
    std::vector<uid_t> events;
    for each (std::pair<uid_t,uid_t> pair in in_map) {
      peers.push_back(pair.first);
      events.push_back(pair.second);
    }
    if (peers.size() > 0) {
      std::copy(peers.begin(), peers.end(), peers_list);
    }
    if (events.size() > 0) {
      std::copy(events.begin(), events.end(), events_list);
    }
  }

  sgx_status_t e_provision_enclave(uid_t peer_id, dcr_workflow workflow, std::map<uid_t, uid_t, cmp_uids> peer_to_event_map) {
    intermediate_dcr_worflow temp = workflow.create_intermediate();
    uid_t* peer_map_peers = new uid_t[peer_to_event_map.size()];
    uid_t* peer_map_events = new uid_t[peer_to_event_map.size()];
    flatten_map(peer_to_event_map, peer_map_peers, peer_map_events);
    uint32_t peer_map_count = peer_to_event_map.size();
    sgx_status_t status = provision_enclave(
      eid,
      peer_id,                /* self_id */
      temp.event_ids, /* event_ids */
      temp.events_count, /* events_count */
      temp.excluded, /* excluded */
      temp.excluded_count, /* excluded_count */
      temp.pending, /* pending */
      temp.pending_count, /* pending_count */
      temp.executed, /* executed */
      temp.executed_count, /* executed_count */
      temp.dcr_conditions_out,/* dcr_conditions_out */
      temp.dcr_conditions_in,/* dcr_conditions_in */
      temp.conditions_count, /* conditions_count */
      temp.dcr_milestones_out,/* dcr_milestones_out */
      temp.dcr_milestones_in,/* dcr_milestones_in */
      temp.milestones_count, /* milestones_count */
      temp.dcr_includes_out, /* dcr_includes_out */
      temp.dcr_includes_in, /* dcr_includes_in */
      temp.includes_count, /* includes_count */
      temp.dcr_excludes_out,/* dcr_excludes_out */
      temp.dcr_excludes_in,/* dcr_excludes_in */
      temp.excludes_count,/* excludes_count */
      temp.dcr_responses_out, /* dcr_responses_out */
      temp.dcr_responses_in,/* dcr_responses_in */
      temp.responses_count, /* responses_count */
      peer_map_peers,/* peer_map_peers */
      peer_map_events,/* peer_map_events */
      peer_to_event_map.size()/* peer_map_count */
    );

    delete peer_map_events;
    delete peer_map_peers;

    if (status != SGX_SUCCESS)
      return status;

    return e_set_time();
  }

#ifdef SGX_DEBUG
  void e_test_execute(uid_t event_id) {
    sgx_status_t status = test_execute(eid, event_id);
  }

  bool e_test_is_enabled(uid_t event_id) {
    bool ret;
    sgx_status_t status = test_is_enabled(eid, &ret, event_id);
    return ret;
  }

  bool e_test_is_executed(uid_t event_id) {
    bool ret;
    sgx_status_t status = test_is_executed(eid, &ret, event_id);
    return ret;
  }

  bool e_test_is_pending(uid_t event_id) {
    bool ret;
    sgx_status_t status = test_is_pending(eid, &ret, event_id);
    return ret;
  }

  bool e_test_is_excluded(uid_t event_id) {
    bool ret;
    sgx_status_t status = test_is_excluded(eid, &ret, event_id);
    return ret;
  }

  command_req_t e_test_set_mac_command_req(command_req_t req) {
    command_req_t ret;
    sgx_status_t status = test_set_mac_command_req(eid, &ret, req);
    return ret;
  }

  command_rsp_t e_test_set_mac_command_rsp(command_rsp_t rsp) {
    command_rsp_t ret;
    sgx_status_t status = test_set_mac_command_rsp(eid, &ret, rsp);
    return ret;
  }

  append_req_t e_test_set_mac_append_req(append_req_t req) {
    append_req_t ret;
    sgx_status_t status = test_set_mac_append_req(eid, &ret, req);
    return ret;
  }

  append_rsp_t e_test_set_mac_append_rsp(append_rsp_t rsp) {
    append_rsp_t ret;
    sgx_status_t status = test_set_mac_append_rsp(eid, &ret, rsp);
    return ret;
  }

  poll_req_t e_test_set_mac_poll_req(poll_req_t req) {
    poll_req_t ret;
    sgx_status_t status = test_set_mac_poll_req(eid, &ret, req);
    return ret;
  }

  poll_rsp_t e_test_set_mac_poll_rsp(poll_rsp_t rsp) {
    poll_rsp_t ret;
    sgx_status_t status = test_set_mac_poll_rsp(eid, &ret, rsp);
    return ret;
  }

  election_req_t e_test_set_mac_election_req(election_req_t req) {
    election_req_t ret;
    sgx_status_t status = test_set_mac_election_req(eid, &ret, req);
    return ret;
  }

  election_rsp_t e_test_set_mac_election_rsp(election_rsp_t rsp) {
    election_rsp_t ret;
    sgx_status_t status = test_set_mac_election_rsp(eid, &ret, rsp);
    return ret;
  }

  bool e_test_verify_mac_poll_req(poll_req_t req) {
    bool ret;
    sgx_status_t status = test_verify_mac_poll_req(eid, &ret, req);
    return ret;
  }

  uid_t e_test_leader_of_event(uid_t event_id) {
    uid_t ret;
    sgx_status_t status = test_leader_of_event(eid, &ret, event_id);
    return ret;
  }

  uid_t e_test_event_of_peer(uid_t peer_id) {
    uid_t ret;
    sgx_status_t status = test_event_of_peer(eid, &ret, peer_id);
    return ret;
  }

  uint32_t e_test_size_of_event_cluster() {
    uint32_t ret;
    sgx_status_t status = test_size_of_event_cluster(eid, &ret);
    return ret;
  }

  bool e_test_is_leader() {
    bool ret;
    sgx_status_t status = test_is_leader(eid, &ret);
    return ret;
  }
#endif
};
