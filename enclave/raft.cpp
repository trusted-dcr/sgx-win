#include <iterator>
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
  endorsement = empty_uid;
  poll_votes = 1;
  election_votes = 1;
  leader = new_leader;
	leader_map[cluster_event] = new_leader;
}

bool peer::term_and_index_exist_in_log(uint32_t term, uint32_t index) {
  if (log.size() <= index)
    return false;
  entry_t entry_i = log[index];
  return entry_i.term == term;
}


bool peer::exist_in_log(command_tag_t tag) {
  uint32_t placeholder;
  return exist_in_log(tag, placeholder);
}

bool peer::exist_in_log(command_tag_t tag, uint32_t& ret_index) {
  for each (entry_t logged in log) {
    if (tags_equal(logged.tag, tag)) {
      ret_index = logged.index;
      return true;
    }
  }
  return false;
}

void peer::set_commit_index(uint32_t new_index) {
  commit_index = new_index;
}

uint32_t peer::get_commit_index() {
  return commit_index;
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
  uint32_t index = indices_vector[(indices_vector.size() - 1) / 2];

  return index;
}

bool peer::executing_own_event() {
  return uids_equal(log[locked_entry_index].event, cluster_event);
}

bool peer::locks_aquired() {
  std::set<uid_t, cmp_uids> lock_set = workflow.get_lock_set(cluster_event);
  for each (uid_t lock in lock_set) {
    if (locked_events.find(lock) == locked_events.end())
      return false;
  }
  return true;
}

bool peer::lock_is_resolved(uint32_t index) {
  entry_t lock = log[index];
  if (lock.tag.type != LOCK) { //if it's not a lock, it's trivially resolved
    return true;
  }
  for (uint32_t i = index+1; i < commit_index+1; i++) {
    if(log[i].tag.type != LOCK) // should never be false
      if (uids_equal(lock.event, log[i].event))
        return true;
  }
  return false;
}

bool peer::has_unresolved_lock(uint32_t& out_val) {
  for (uint32_t i = last_checkpoint; i < commit_index + 1; i++) {
    entry_t entry = log[i];
    if (entry.tag.type == LOCK) {
      if (!lock_is_resolved(i)) {
        out_val = i;
        return true;
      }
    }
  }
  return false;
}

bool peer::has_unresolved_lock() {
  uint32_t placeholder;
  return has_unresolved_lock(placeholder);
}

bool peer::lock_resolve_is_committed(uint32_t entry_id, entry_t out_entry) {
  entry_t lock = log[entry_id];
  if (lock.tag.type != LOCK) { //if it's not a lock, there is no next entry
    return false;
  }
  for (uint32_t i = entry_id + 1; i < commit_index +1; i++) {
    if (uids_equal(log[i].event, lock.event)) {
      out_entry = log[i];
      return true;
    }
  }
  return false;
}

uint32_t peer::find_latest_checkpoint() {
  for (int i = log.size() - 1; i >= 0; i--) {
    entry_t entry = log[i];
    if (entry.tag.type == CHECKPOINT)
      return i;
  }
  return 0;
}

uid_t peer::find_other_peer_in_cluster(uid_t old_source) {
  //first find the relevant cluster
  uid_t event_id = { 0 };
  for each (std::pair<uid_t, uid_t> peer_to_event in peer_to_event_map) {
    if (uids_equal(peer_to_event.first, old_source)) {
      event_id = peer_to_event.second;
    }
  }

  //then find all peers in the same cluster
  std::vector<uid_t> cluster_peers;
  for each (std::pair<uid_t,uid_t> peer_to_event in peer_to_event_map) {
    if (uids_equal(peer_to_event.second, event_id) &&
      !uids_equal(peer_to_event.first, old_source) &&
      !uids_equal(peer_to_event.first, id)) //this last check should never happen
    {
      cluster_peers.push_back(peer_to_event.first);
    }
  }
  if(cluster_peers.size() == 0)
    return old_source; //no alternatives found

  //then choose a random new peer
  uint64_t buf;
  sgx_read_rand((unsigned char*)&buf, 4);
  buf = buf % cluster_peers.size();

  return cluster_peers[buf];
}


