#include "enclave_u.h"
#include "daemon.h"

extern daemon* daemon_instance;

void send_command_req(command_req_t req) {
  daemon_instance->async_send(req);
  std::cout << "[INFO] Sending <COMMAND_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
}

void send_command_rsp(command_rsp_t rsp) {
  daemon_instance->async_send(rsp);
  std::cout << "[INFO] Sending <COMMAND_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
}

void send_append_req(append_req_t req, entry_t* entries, int size) {
  req.entries = entries;
  daemon_instance->async_send(req);
  std::cout << "[INFO] Sending <APPEND_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
}

void send_append_rsp(append_rsp_t rsp) {
  daemon_instance->async_send(rsp);
  std::cout << "[INFO] Sending <APPEND_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
}

void send_poll_req(poll_req_t req) {
  daemon_instance->async_send(req);
  std::cout
		<< "[INFO] Sending <POLL_REQUEST, "
		<< req.term << ", "
		<< req.last_term << ", "
		<< req.last_index
		<< "> to "
		<< daemon_instance->addrs[req.target] << std::endl;
}

void send_poll_rsp(poll_rsp_t rsp) {
  daemon_instance->async_send(rsp);
  std::cout
		<< "[INFO] Sending <POLL_RESPONSE, "
		<< rsp.term << ", "
		<< rsp.success
    << "> to " << daemon_instance->addrs[rsp.target] << std::endl;
}

void send_election_req(election_req_t req) {
  daemon_instance->async_send(req);
  std::cout << "[INFO] Sending <ELECTION_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
}

void send_election_rsp(election_rsp_t rsp) {
  daemon_instance->async_send(rsp);
  std::cout << "[INFO] Sending <ELECTION_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
}

void send_log_req(log_req_t req) {
  daemon_instance->async_send(req);
  std::cout << "[INFO] Sending <LOG_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
}

void send_log_rsp(log_rsp_t rsp, entry_t* entries, int size) {
	rsp.entries = entries;
  daemon_instance->async_send(rsp);
  std::cout << "[INFO] Sending <LOG_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
}

void print(const char* str) {
	std::cout << str;
}

void return_history(entry_t* entries, uint32_t size, uid_t cluster) {
	// timed out
	if (!daemon_instance->history_in_progress)
		return;
	
	for (size_t i = 0; i < size; i++)
		daemon_instance->history[cluster].push_back(entries[i]);

	// if we get answers from all clusters, we can stop early
	if (daemon_instance->history.size() == daemon_instance->wf_size)
		daemon_instance->history_ready = true;
}
