#pragma once
#include <map>
#include <vector>
#include "sgx_tae_service.h"
#include "msg.h"
#include "msg_util.h"

enum role_t { FOLLOWER, CANDIDATE, LEADER };

const uid_t empty_uid = { 0 };

class peer {
public:
  uid_t id;
  role_t role;
  std::vector<entry_t> log;
  uint32_t term;
  uint32_t commit_index;
  std::map<uid_t, uint32_t, cmp_uids> indices;
  uint64_t min_timeout;
  uint64_t timeout;
  uid_t endorsement;
  uint32_t poll_votes;
  uint32_t election_votes;
  uid_t leader;
  uint32_t cluster_size;
  std::vector<uid_t> cluster_members;

  void init();

  entry_t last_entry();

  void remove_elements_after(uint32_t index);

  void update_term(uint32_t new_term, uid_t new_leader);

  bool exist_in_log(uint32_t term, uint32_t index);
  bool exist_in_log(command_tag_t tag, entry_t entry);

  uint32_t largest_majority_index();
};



