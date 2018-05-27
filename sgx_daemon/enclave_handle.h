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
    peers_list = &peers[0];
    events_list = &events[0];
  }

  sgx_status_t e_provision_enclave(uid_t peer_id, dcr_workflow workflow, uid_t cluster_event_id, std::map<uid_t, uid_t, cmp_uids> peer_to_event_map) {
    uid_t wf_id = workflow.id;
    intermediate_dcr_worflow temp = workflow.create_intermediate();
    uid_t* peer_map_peers;
    uid_t* peer_map_events;
    flatten_map(peer_to_event_map, peer_map_peers, peer_map_events);
    uint32_t peer_map_count = peer_to_event_map.size();
    sgx_status_t status = provision_enclave(
      eid,        
      peer_id,                /* self_id */
      cluster_event_id,       /* self_cluster_event */
      wf_id,                  /* wf_id */
      (char*)workflow.name.c_str(),  /* wf_name */
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
    if (status != SGX_SUCCESS)
      return status;
    return e_set_time();
  }
};
