#include "raft.h"
//
//void open_pse() {
//  sgx_status_t ret;
//  do {
//    ret = sgx_create_pse_session();
//  } while (ret != SGX_SUCCESS);
//}
//
//void close_pse() {
//  sgx_status_t ret;
//  do {
//    ret = sgx_close_pse_session();
//  } while (ret != SGX_SUCCESS);
//}


void peer::init() {
  update_term(0, { 0 });
  commit_index = 0;
  //for each (uid_t id in peers) {
  //  indices[id] = 0;
  //}
}

entry_t peer::last_entry() {
  size_t size = log.size();
  return log[size - 1];
}

void peer::update_term(uint32_t new_term, uid_t new_leader) {
  term = new_term;
  endorsement = { 0 };
  poll_votes = 1;
  election_votes = 1;
  leader = new_leader;
}

bool peer::exist_in_log(uint32_t term, uint32_t index) {
  if (log.size() >= index)
    return false;
  entry_t entry_i = log[index];
  return entry_i.term == term;
}

bool peer::exist_in_log(command_tag_t tag, entry_t entry) {
  for each (entry_t logged in peer::log) {
    if (tags_equal(logged.tag, entry.tag)) {
      if (entry_equal(logged, entry))
        return true;
    }
    return false;
  }

}

void peer::remove_elements_after(uint32_t index) {
  while (log.size() > index + 1) {
    log.pop_back();
  }
}


uint32_t peer::largest_majority_index() {
  std::vector<uint32_t> indices_vector;
  for each (std::pair<uid_t, uint32_t> index_pair in indices) {
    indices_vector.push_back(index_pair.second);
  }

  std::sort(indices_vector.begin(), indices_vector.end());

  uint32_t index = indices_vector[indices_vector.size() / 2 - 1];
 
  return index;
}
