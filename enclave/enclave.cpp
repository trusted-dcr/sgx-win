#include <string>
#include "enclave_t.h"
#include "msg.h"

using namespace std;

void recv_command_req(command_req_t req) {

}

void recv_command_rsp(command_rsp_t rsp) {

}

void recv_append_req(append_req_t req) {
	// copy entries to protected memory
	entry_t* prot_entries = new entry_t[req.entries_n];
	memcpy(req.entries, prot_entries, sizeof(entry_t) * req.entries_n);
	req.entries = prot_entries;
}

void recv_append_rsp(append_rsp_t rsp) {

}

void recv_poll_req(poll_req_t req) {

}

void recv_poll_rsp(poll_rsp_t rsp) {

}

void recv_election_req(election_req_t req) {

}

void recv_election_rsp(election_rsp_t rsp) {

}

