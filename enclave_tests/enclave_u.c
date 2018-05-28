#include "enclave_u.h"
#include <errno.h>

typedef struct ms_set_time_t {
	uint64_t ms_ticks;
} ms_set_time_t;

typedef struct ms_recv_command_req_t {
	command_req_t ms_req;
} ms_recv_command_req_t;

typedef struct ms_recv_command_rsp_t {
	command_rsp_t ms_rsp;
} ms_recv_command_rsp_t;

typedef struct ms_recv_append_req_t {
	append_req_t ms_req;
} ms_recv_append_req_t;

typedef struct ms_recv_append_rsp_t {
	append_rsp_t ms_rsp;
} ms_recv_append_rsp_t;

typedef struct ms_recv_poll_req_t {
	poll_req_t ms_req;
} ms_recv_poll_req_t;

typedef struct ms_recv_poll_rsp_t {
	poll_rsp_t ms_rsp;
} ms_recv_poll_rsp_t;

typedef struct ms_recv_election_req_t {
	election_req_t ms_req;
} ms_recv_election_req_t;

typedef struct ms_recv_election_rsp_t {
	election_rsp_t ms_rsp;
} ms_recv_election_rsp_t;

typedef struct ms_provision_enclave_t {
	uid_t ms_self_id;
	uid_t ms_wf_id;
	char* ms_wf_name;
	size_t ms_wf_name_len;
	uid_t* ms_event_ids;
	uint32_t ms_events_count;
	uid_t* ms_excluded;
	uint32_t ms_excluded_count;
	uid_t* ms_pending;
	uint32_t ms_pending_count;
	uid_t* ms_executed;
	uint32_t ms_executed_count;
	uid_t* ms_dcr_conditions_out;
	uid_t* ms_dcr_conditions_in;
	uint32_t ms_conditions_count;
	uid_t* ms_dcr_milestones_out;
	uid_t* ms_dcr_milestones_in;
	uint32_t ms_milestones_count;
	uid_t* ms_dcr_includes_out;
	uid_t* ms_dcr_includes_in;
	uint32_t ms_includes_count;
	uid_t* ms_dcr_excludes_out;
	uid_t* ms_dcr_excludes_in;
	uint32_t ms_excludes_count;
	uid_t* ms_dcr_responses_out;
	uid_t* ms_dcr_responses_in;
	uint32_t ms_responses_count;
	uid_t* ms_peer_map_peers;
	uid_t* ms_peer_map_events;
	uint32_t ms_peer_map_count;
} ms_provision_enclave_t;

typedef struct ms_test_set_mac_command_req_t {
	command_req_t ms_retval;
	command_req_t ms_req;
} ms_test_set_mac_command_req_t;

typedef struct ms_test_set_mac_command_rsp_t {
	command_rsp_t ms_retval;
	command_rsp_t ms_rsp;
} ms_test_set_mac_command_rsp_t;

typedef struct ms_test_set_mac_append_req_t {
	append_req_t ms_retval;
	append_req_t ms_req;
} ms_test_set_mac_append_req_t;

typedef struct ms_test_set_mac_append_rsp_t {
	append_rsp_t ms_retval;
	append_rsp_t ms_rsp;
} ms_test_set_mac_append_rsp_t;

typedef struct ms_test_set_mac_poll_req_t {
	poll_req_t ms_retval;
	poll_req_t ms_req;
} ms_test_set_mac_poll_req_t;

typedef struct ms_test_set_mac_poll_rsp_t {
	poll_rsp_t ms_retval;
	poll_rsp_t ms_rsp;
} ms_test_set_mac_poll_rsp_t;

typedef struct ms_test_set_mac_election_req_t {
	election_req_t ms_retval;
	election_req_t ms_req;
} ms_test_set_mac_election_req_t;

typedef struct ms_test_set_mac_election_rsp_t {
	election_rsp_t ms_retval;
	election_rsp_t ms_rsp;
} ms_test_set_mac_election_rsp_t;

typedef struct ms_test_verify_mac_poll_req_t {
	bool ms_retval;
	poll_req_t ms_req;
} ms_test_verify_mac_poll_req_t;

typedef struct ms_test_execute_t {
	uid_t ms_event_id;
} ms_test_execute_t;

typedef struct ms_test_is_enabled_t {
	bool ms_retval;
	uid_t ms_event_id;
} ms_test_is_enabled_t;

typedef struct ms_test_is_executed_t {
	bool ms_retval;
	uid_t ms_event_id;
} ms_test_is_executed_t;

typedef struct ms_test_is_pending_t {
	bool ms_retval;
	uid_t ms_event_id;
} ms_test_is_pending_t;

typedef struct ms_test_is_excluded_t {
	bool ms_retval;
	uid_t ms_event_id;
} ms_test_is_excluded_t;

typedef struct ms_test_leader_of_event_t {
	uid_t ms_retval;
	uid_t ms_event_id;
} ms_test_leader_of_event_t;

typedef struct ms_test_event_of_peer_t {
	uid_t ms_retval;
	uid_t ms_peer_id;
} ms_test_event_of_peer_t;

typedef struct ms_test_size_of_event_cluster_t {
	uint32_t ms_retval;
} ms_test_size_of_event_cluster_t;

typedef struct ms_test_is_leader_t {
	bool ms_retval;
} ms_test_is_leader_t;

typedef struct ms_send_command_req_t {
	command_req_t ms_req;
} ms_send_command_req_t;

typedef struct ms_send_command_rsp_t {
	command_rsp_t ms_rsp;
} ms_send_command_rsp_t;

typedef struct ms_send_append_req_t {
	append_req_t ms_req;
	entry_t* ms_entries;
	int ms_size;
} ms_send_append_req_t;

typedef struct ms_send_append_rsp_t {
	append_rsp_t ms_rsp;
} ms_send_append_rsp_t;

typedef struct ms_send_poll_req_t {
	poll_req_t ms_req;
} ms_send_poll_req_t;

typedef struct ms_send_poll_rsp_t {
	poll_rsp_t ms_rsp;
} ms_send_poll_rsp_t;

typedef struct ms_send_election_req_t {
	election_req_t ms_req;
} ms_send_election_req_t;

typedef struct ms_send_election_rsp_t {
	election_rsp_t ms_rsp;
} ms_send_election_rsp_t;

static sgx_status_t SGX_CDECL enclave_send_command_req(void* pms)
{
	ms_send_command_req_t* ms = SGX_CAST(ms_send_command_req_t*, pms);
	send_command_req(ms->ms_req);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_command_rsp(void* pms)
{
	ms_send_command_rsp_t* ms = SGX_CAST(ms_send_command_rsp_t*, pms);
	send_command_rsp(ms->ms_rsp);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_append_req(void* pms)
{
	ms_send_append_req_t* ms = SGX_CAST(ms_send_append_req_t*, pms);
	send_append_req(ms->ms_req, ms->ms_entries, ms->ms_size);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_append_rsp(void* pms)
{
	ms_send_append_rsp_t* ms = SGX_CAST(ms_send_append_rsp_t*, pms);
	send_append_rsp(ms->ms_rsp);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_poll_req(void* pms)
{
	ms_send_poll_req_t* ms = SGX_CAST(ms_send_poll_req_t*, pms);
	send_poll_req(ms->ms_req);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_poll_rsp(void* pms)
{
	ms_send_poll_rsp_t* ms = SGX_CAST(ms_send_poll_rsp_t*, pms);
	send_poll_rsp(ms->ms_rsp);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_election_req(void* pms)
{
	ms_send_election_req_t* ms = SGX_CAST(ms_send_election_req_t*, pms);
	send_election_req(ms->ms_req);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_send_election_rsp(void* pms)
{
	ms_send_election_rsp_t* ms = SGX_CAST(ms_send_election_rsp_t*, pms);
	send_election_rsp(ms->ms_rsp);

	return SGX_SUCCESS;
}

static sgx_status_t SGX_CDECL enclave_request_time(void* pms)
{
	if (pms != NULL) return SGX_ERROR_INVALID_PARAMETER;
	request_time();
	return SGX_SUCCESS;
}

static const struct {
	size_t nr_ocall;
	void * func_addr[9];
} ocall_table_enclave = {
	9,
	{
		(void*)(uintptr_t)enclave_send_command_req,
		(void*)(uintptr_t)enclave_send_command_rsp,
		(void*)(uintptr_t)enclave_send_append_req,
		(void*)(uintptr_t)enclave_send_append_rsp,
		(void*)(uintptr_t)enclave_send_poll_req,
		(void*)(uintptr_t)enclave_send_poll_rsp,
		(void*)(uintptr_t)enclave_send_election_req,
		(void*)(uintptr_t)enclave_send_election_rsp,
		(void*)(uintptr_t)enclave_request_time,
	}
};

sgx_status_t set_time(sgx_enclave_id_t eid, uint64_t ticks)
{
	sgx_status_t status;
	ms_set_time_t ms;
	ms.ms_ticks = ticks;
	status = sgx_ecall(eid, 0, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_command_req(sgx_enclave_id_t eid, command_req_t req)
{
	sgx_status_t status;
	ms_recv_command_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 1, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_command_rsp(sgx_enclave_id_t eid, command_rsp_t rsp)
{
	sgx_status_t status;
	ms_recv_command_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 2, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_append_req(sgx_enclave_id_t eid, append_req_t req)
{
	sgx_status_t status;
	ms_recv_append_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 3, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_append_rsp(sgx_enclave_id_t eid, append_rsp_t rsp)
{
	sgx_status_t status;
	ms_recv_append_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 4, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_poll_req(sgx_enclave_id_t eid, poll_req_t req)
{
	sgx_status_t status;
	ms_recv_poll_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 5, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_poll_rsp(sgx_enclave_id_t eid, poll_rsp_t rsp)
{
	sgx_status_t status;
	ms_recv_poll_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 6, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_election_req(sgx_enclave_id_t eid, election_req_t req)
{
	sgx_status_t status;
	ms_recv_election_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 7, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t recv_election_rsp(sgx_enclave_id_t eid, election_rsp_t rsp)
{
	sgx_status_t status;
	ms_recv_election_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 8, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t provision_enclave(sgx_enclave_id_t eid, uid_t self_id, uid_t wf_id, char* wf_name, uid_t* event_ids, uint32_t events_count, uid_t* excluded, uint32_t excluded_count, uid_t* pending, uint32_t pending_count, uid_t* executed, uint32_t executed_count, uid_t* dcr_conditions_out, uid_t* dcr_conditions_in, uint32_t conditions_count, uid_t* dcr_milestones_out, uid_t* dcr_milestones_in, uint32_t milestones_count, uid_t* dcr_includes_out, uid_t* dcr_includes_in, uint32_t includes_count, uid_t* dcr_excludes_out, uid_t* dcr_excludes_in, uint32_t excludes_count, uid_t* dcr_responses_out, uid_t* dcr_responses_in, uint32_t responses_count, uid_t* peer_map_peers, uid_t* peer_map_events, uint32_t peer_map_count)
{
	sgx_status_t status;
	ms_provision_enclave_t ms;
	ms.ms_self_id = self_id;
	ms.ms_wf_id = wf_id;
	ms.ms_wf_name = (char*)wf_name;
	ms.ms_wf_name_len = wf_name ? strlen(wf_name) + 1 : 0;
	ms.ms_event_ids = event_ids;
	ms.ms_events_count = events_count;
	ms.ms_excluded = excluded;
	ms.ms_excluded_count = excluded_count;
	ms.ms_pending = pending;
	ms.ms_pending_count = pending_count;
	ms.ms_executed = executed;
	ms.ms_executed_count = executed_count;
	ms.ms_dcr_conditions_out = dcr_conditions_out;
	ms.ms_dcr_conditions_in = dcr_conditions_in;
	ms.ms_conditions_count = conditions_count;
	ms.ms_dcr_milestones_out = dcr_milestones_out;
	ms.ms_dcr_milestones_in = dcr_milestones_in;
	ms.ms_milestones_count = milestones_count;
	ms.ms_dcr_includes_out = dcr_includes_out;
	ms.ms_dcr_includes_in = dcr_includes_in;
	ms.ms_includes_count = includes_count;
	ms.ms_dcr_excludes_out = dcr_excludes_out;
	ms.ms_dcr_excludes_in = dcr_excludes_in;
	ms.ms_excludes_count = excludes_count;
	ms.ms_dcr_responses_out = dcr_responses_out;
	ms.ms_dcr_responses_in = dcr_responses_in;
	ms.ms_responses_count = responses_count;
	ms.ms_peer_map_peers = peer_map_peers;
	ms.ms_peer_map_events = peer_map_events;
	ms.ms_peer_map_count = peer_map_count;
	status = sgx_ecall(eid, 9, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t test_set_mac_command_req(sgx_enclave_id_t eid, command_req_t* retval, command_req_t req)
{
	sgx_status_t status;
	ms_test_set_mac_command_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 10, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_command_rsp(sgx_enclave_id_t eid, command_rsp_t* retval, command_rsp_t rsp)
{
	sgx_status_t status;
	ms_test_set_mac_command_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 11, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_append_req(sgx_enclave_id_t eid, append_req_t* retval, append_req_t req)
{
	sgx_status_t status;
	ms_test_set_mac_append_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 12, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_append_rsp(sgx_enclave_id_t eid, append_rsp_t* retval, append_rsp_t rsp)
{
	sgx_status_t status;
	ms_test_set_mac_append_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 13, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_poll_req(sgx_enclave_id_t eid, poll_req_t* retval, poll_req_t req)
{
	sgx_status_t status;
	ms_test_set_mac_poll_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 14, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_poll_rsp(sgx_enclave_id_t eid, poll_rsp_t* retval, poll_rsp_t rsp)
{
	sgx_status_t status;
	ms_test_set_mac_poll_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 15, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_election_req(sgx_enclave_id_t eid, election_req_t* retval, election_req_t req)
{
	sgx_status_t status;
	ms_test_set_mac_election_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 16, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_set_mac_election_rsp(sgx_enclave_id_t eid, election_rsp_t* retval, election_rsp_t rsp)
{
	sgx_status_t status;
	ms_test_set_mac_election_rsp_t ms;
	ms.ms_rsp = rsp;
	status = sgx_ecall(eid, 17, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_verify_mac_poll_req(sgx_enclave_id_t eid, bool* retval, poll_req_t req)
{
	sgx_status_t status;
	ms_test_verify_mac_poll_req_t ms;
	ms.ms_req = req;
	status = sgx_ecall(eid, 18, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_execute(sgx_enclave_id_t eid, uid_t event_id)
{
	sgx_status_t status;
	ms_test_execute_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 19, &ocall_table_enclave, &ms);
	return status;
}

sgx_status_t test_is_enabled(sgx_enclave_id_t eid, bool* retval, uid_t event_id)
{
	sgx_status_t status;
	ms_test_is_enabled_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 20, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_is_executed(sgx_enclave_id_t eid, bool* retval, uid_t event_id)
{
	sgx_status_t status;
	ms_test_is_executed_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 21, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_is_pending(sgx_enclave_id_t eid, bool* retval, uid_t event_id)
{
	sgx_status_t status;
	ms_test_is_pending_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 22, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_is_excluded(sgx_enclave_id_t eid, bool* retval, uid_t event_id)
{
	sgx_status_t status;
	ms_test_is_excluded_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 23, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_leader_of_event(sgx_enclave_id_t eid, uid_t* retval, uid_t event_id)
{
	sgx_status_t status;
	ms_test_leader_of_event_t ms;
	ms.ms_event_id = event_id;
	status = sgx_ecall(eid, 24, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_event_of_peer(sgx_enclave_id_t eid, uid_t* retval, uid_t peer_id)
{
	sgx_status_t status;
	ms_test_event_of_peer_t ms;
	ms.ms_peer_id = peer_id;
	status = sgx_ecall(eid, 25, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_size_of_event_cluster(sgx_enclave_id_t eid, uint32_t* retval)
{
	sgx_status_t status;
	ms_test_size_of_event_cluster_t ms;
	status = sgx_ecall(eid, 26, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

sgx_status_t test_is_leader(sgx_enclave_id_t eid, bool* retval)
{
	sgx_status_t status;
	ms_test_is_leader_t ms;
	status = sgx_ecall(eid, 27, &ocall_table_enclave, &ms);
	if (status == SGX_SUCCESS && retval) *retval = ms.ms_retval;
	return status;
}

