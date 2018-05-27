#include "enclave_u.h"
#include "daemon.h"

void send_command_req(command_req_t req) {
  std::cout << "[INFO] <COMMAND_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
  daemon_instance->async_send(req);
}

void send_command_rsp(command_rsp_t rsp) {
  std::cout << "[INFO] <COMMAND_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}

void send_append_req(append_req_t req, entry_t* entries, int size) {
  std::cout << "[INFO] <APPEND_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
  req.entries = entries;
  daemon_instance->async_send(req);
}

void send_append_rsp(append_rsp_t rsp) {
  std::cout << "[INFO] <APPEND_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}

void send_poll_req(poll_req_t req) {
  std::cout << "[INFO] <POLL_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
  daemon_instance->async_send(req);
}

void send_poll_rsp(poll_rsp_t rsp) {
  std::cout << "[INFO] <POLL_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}

void send_election_req(election_req_t req) {
  std::cout << "[INFO] <ELECTION_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
  daemon_instance->async_send(req);
}

void send_election_rsp(election_rsp_t rsp) {
  std::cout << "[INFO] <ELECTION_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}

void send_log_req(log_req_t req) {
  std::cout << "[INFO] <LOG_REQUEST> to " << daemon_instance->addrs[req.target] << std::endl;
  daemon_instance->async_send(req);
}

void send_log_rsp(log_rsp_t rsp) {
  std::cout << "[INFO] <LOG_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}
