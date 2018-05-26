#pragma once
#include <tchar.h>
#include "sgx_urts.h"
#include "enclave_u.h"

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
};
