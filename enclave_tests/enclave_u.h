#ifndef ENCLAVE_U_H__
#define ENCLAVE_U_H__

#include <stdint.h>
#include <wchar.h>
#include <stddef.h>
#include <string.h>
#include "sgx_edger8r.h" /* for sgx_status_t etc. */

#include "msg.h"

#define SGX_CAST(type, item) ((type)(item))

#ifdef __cplusplus
extern "C" {
#endif

void SGX_UBRIDGE(SGX_NOCONVENTION, send_command_req, (command_req_t req));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_command_rsp, (command_rsp_t rsp));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_append_req, (append_req_t req, entry_t* entries, int size));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_append_rsp, (append_rsp_t rsp));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_poll_req, (poll_req_t req));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_poll_rsp, (poll_rsp_t rsp));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_election_req, (election_req_t req));
void SGX_UBRIDGE(SGX_NOCONVENTION, send_election_rsp, (election_rsp_t rsp));
void SGX_UBRIDGE(SGX_NOCONVENTION, request_time, ());

sgx_status_t set_time(sgx_enclave_id_t eid, uint64_t ticks);
sgx_status_t recv_command_req(sgx_enclave_id_t eid, command_req_t req);
sgx_status_t recv_command_rsp(sgx_enclave_id_t eid, command_rsp_t rsp);
sgx_status_t recv_append_req(sgx_enclave_id_t eid, append_req_t req);
sgx_status_t recv_append_rsp(sgx_enclave_id_t eid, append_rsp_t rsp);
sgx_status_t recv_poll_req(sgx_enclave_id_t eid, poll_req_t req);
sgx_status_t recv_poll_rsp(sgx_enclave_id_t eid, poll_rsp_t rsp);
sgx_status_t recv_election_req(sgx_enclave_id_t eid, election_req_t req);
sgx_status_t recv_election_rsp(sgx_enclave_id_t eid, election_rsp_t rsp);
sgx_status_t provision_enclave(sgx_enclave_id_t eid, uid_t self_id, uid_t self_cluster_event, uid_t wf_id, char* wf_name, uid_t* event_ids, uint32_t events_count, uid_t* excluded, uint32_t excluded_count, uid_t* pending, uint32_t pending_count, uid_t* executed, uint32_t executed_count, uid_t* dcr_conditions_out, uid_t* dcr_conditions_in, uint32_t conditions_count, uid_t* dcr_milestones_out, uid_t* dcr_milestones_in, uint32_t milestones_count, uid_t* dcr_includes_out, uid_t* dcr_includes_in, uint32_t includes_count, uid_t* dcr_excludes_out, uid_t* dcr_excludes_in, uint32_t excludes_count, uid_t* dcr_responses_out, uid_t* dcr_responses_in, uint32_t responses_count, uid_t* peer_map_peers, uid_t* peer_map_events, uint32_t peer_map_count);
sgx_status_t test_set_mac_command_req(sgx_enclave_id_t eid, command_req_t* retval, command_req_t req);
sgx_status_t test_set_mac_command_rsp(sgx_enclave_id_t eid, command_rsp_t* retval, command_rsp_t rsp);
sgx_status_t test_set_mac_append_req(sgx_enclave_id_t eid, append_req_t* retval, append_req_t req);
sgx_status_t test_set_mac_append_rsp(sgx_enclave_id_t eid, append_rsp_t* retval, append_rsp_t rsp);
sgx_status_t test_set_mac_poll_req(sgx_enclave_id_t eid, poll_req_t* retval, poll_req_t req);
sgx_status_t test_set_mac_poll_rsp(sgx_enclave_id_t eid, poll_rsp_t* retval, poll_rsp_t rsp);
sgx_status_t test_set_mac_election_req(sgx_enclave_id_t eid, election_req_t* retval, election_req_t req);
sgx_status_t test_set_mac_election_rsp(sgx_enclave_id_t eid, election_rsp_t* retval, election_rsp_t rsp);
sgx_status_t test_execute(sgx_enclave_id_t eid, uid_t event_id);
sgx_status_t test_is_enabled(sgx_enclave_id_t eid, bool* retval, uid_t event_id);
sgx_status_t test_is_executed(sgx_enclave_id_t eid, bool* retval, uid_t event_id);
sgx_status_t test_is_pending(sgx_enclave_id_t eid, bool* retval, uid_t event_id);
sgx_status_t test_is_excluded(sgx_enclave_id_t eid, bool* retval, uid_t event_id);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif
