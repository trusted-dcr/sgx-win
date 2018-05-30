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

void return_history(entry_t* flat_entry_list, uint32_t entry_length,
  uid_t* event_uids, uint32_t id_length,
  uint32_t* offset_list) {
  std::map<uid_t, std::vector<entry_t>, cmp_uids> cluster_to_log_map;

  uint32_t acc = 0;
  uint32_t j = 0;
  for (uint32_t i = 0; i < id_length; i++) {
    for (j = 0; j < offset_list[i]; j++) {
      uid_t event_id = event_uids[i];
      entry_t entry = flat_entry_list[acc + j];
      if (cluster_to_log_map.find(event_id) == cluster_to_log_map.end())
				cluster_to_log_map[event_id] = std::vector<entry_t>();
			cluster_to_log_map[event_id].push_back(entry);
    }
    acc = acc + j;
  }

	// if daemon has not timed out yet
	if (daemon_instance->history_in_progress) {
		daemon_instance->history = cluster_to_log_map;
		daemon_instance->history_ready = true;
	}
}
