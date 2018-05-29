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

void send_log_rsp(log_rsp_t rsp, entry_t* entries, int size) {
  std::cout << "[INFO] <LOG_RESPONSE> to " << daemon_instance->addrs[rsp.target] << std::endl;
  daemon_instance->async_send(rsp);
}

void return_logs(entry_t* flat_entry_list, uint32_t entry_length, 
  uid_t* event_uids, uint32_t id_length, 
  uint32_t* offset_list) {
  std::map<uid_t, std::vector<entry_t>, cmp_uids> event_to_log_map;
  
  uint32_t acc = 0;
  uint32_t j = 0;
  for (uint32_t i = 0; i < id_length; i++) {
    for (j = 0; j < offset_list[i]; j++) {
      uid_t event_id = event_uids[i];
      entry_t entry = flat_entry_list[acc + j];
      if (event_to_log_map.find(event_id) == event_to_log_map.end())
        event_to_log_map[event_id] = std::vector<entry_t>();
      event_to_log_map[event_id].push_back(entry);
    }
    acc = acc + j;
  }

  //do something clever
}
