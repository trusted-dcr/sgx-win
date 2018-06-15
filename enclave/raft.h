#pragma once
#include <map>
#include <vector>
#include "sgx_tae_service.h"
#include "msg.h"
#include "msg_util.h"
#include "crypto.h"
#include "dcr.h"


enum role_t { FOLLOWER, CANDIDATE, LEADER };

const uint64_t delta_heartbeat = 100;
const uint64_t delta_min_time = 2*delta_heartbeat;
const uint64_t delta_max_time = 4*delta_heartbeat;
const uint64_t delta_lock_time = 5 * delta_heartbeat;

class peer {
  uint32_t commit_index;
public:
  uid_t id;

  //dcr data
  dcr_workflow workflow;
  uid_t cluster_event;

  // locking and leader data
  std::map<uid_t, uid_t, cmp_uids> peer_to_event_map;
  std::map<uid_t, uid_t, cmp_uids> leader_map;
  std::map<uid_t, command_req_t, cmp_uids> missing_resp;
  std::set<uid_t, cmp_uids> locked_events;
  uint32_t last_checkpoint;

  //misc raft data
  role_t role;
  std::vector<entry_t> log;
  uint32_t term;
  std::map<uid_t, uint32_t, cmp_uids> indices;
  uid_t endorsement;
  uint32_t poll_votes;
  uint32_t election_votes;

  //cluster data
  uid_t leader;
  uint32_t cluster_size;
  std::vector<uid_t> cluster_members;

  //time
  uint64_t t_min;
  uint64_t t;
  uint64_t now;
  bool retried_responses;
  //sgx_thread_mutex_t* time_lock;

  //lock
  int64_t locked_entry_index;

  //logs (for global state collection)
  std::map<uid_t, std::vector<entry_t>, cmp_uids> event_to_log_map;

  void init();

  entry_t last_entry();

  void remove_elements_after(uint32_t index);

  void update_term(uint32_t new_term, uid_t new_leader);

  bool term_and_index_exist_in_log(uint32_t term, uint32_t index);

  bool exist_in_log(command_tag_t tag);

  bool exist_in_log(command_tag_t tag, uint32_t& ret_index);

  void set_commit_index(uint32_t new_index);

  uint32_t get_commit_index();

  uint32_t largest_majority_index();

  bool executing_own_event();

  bool locks_aquired();

  //may not be committed
  bool lock_is_resolved(uint32_t index);

  bool has_unresolved_lock(uint32_t& out_val);

  bool has_unresolved_lock();

  bool lock_resolve_is_committed(uint32_t entry_id, entry_t out_entry);

  uint32_t find_latest_checkpoint();

  uid_t find_other_peer_in_cluster(uid_t old_source);
};



