#include <stdio.h>
#include <tchar.h>
#include <sgx_urts.h>
#include "enclave_u.h"

#define ENCLAVE_FILE _T("enclave.signed.dll")

//sgx_launch_token_t token;
//SecureZeroMemory(token, sizeof(token));
//int updated = 0;
//sgx_enclave_id_t eid = 0;
//sgx_status_t a = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);

void send_command_req(command_req_t req) {

}

void send_command_rsp(command_rsp_t rsp) {

}

void send_append_req(append_req_t req) {

}

void send_append_rsp(append_rsp_t rsp) {

}

void send_poll_req(poll_req_t req) {

}
void send_poll_rsp(poll_rsp_t rsp) {

}

void send_election_req(election_req_t req) {

}
void send_election_rsp(election_rsp_t rsp) {

}

int main() {
	sgx_launch_token_t token;
	SecureZeroMemory(token, sizeof(token));
	int updated = 0;
	sgx_enclave_id_t eid = 0;
	sgx_status_t a = sgx_create_enclave(ENCLAVE_FILE, SGX_DEBUG_FLAG, &token, &updated, &eid, NULL);
}
