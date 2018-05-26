#pragma once
#include <stdint.h>
#include <vector>
#include <map>
#include <set>
#include "msg.h"
#include "msg_util.h"

struct relation_set {
  std::vector<uid_t> excludes;
  std::vector<uid_t> includes;
  std::vector<uid_t> responses;
  std::vector<uid_t> conditions;
  std::vector<uid_t> milestones;
};

struct dcr_event {
  uid_t id;
  std::string name;

  bool executed;
  bool excluded;
  bool pending;

  //relations that effect state or enabledness of this event
  relation_set incoming_relations;

  //relations that this event effect state or enabledness of
  relation_set outgoing_relations;
};

class dcr_workflow {
  uid_t id;
  std::string name;
  std::map<uid_t, dcr_event, cmp_uids> event_store;

public:
  bool is_event_enabled(uid_t event_id);

  void set_event_executed(uid_t event_id);

  std::set<uid_t, cmp_uids> get_lock_set(uid_t event_id);

  std::set<uid_t, cmp_uids> get_inform_set(uid_t event_id);

};
