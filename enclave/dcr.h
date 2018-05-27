#pragma once
#include <stdint.h>
#include <vector>
#include <map>
#include <set>
#include "intermediate_types.h"
#include "msg_util.h"

struct dcr_event {
  uid_t id;

  bool executed;
  bool excluded;
  bool pending;
};

class dcr_workflow {
public:
  uid_t id;
  std::string name;
  std::map<uid_t, dcr_event, cmp_uids> event_store;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> excludes_to; //read as outgoing excludes from given event_id
  std::map<uid_t, std::vector<uid_t>, cmp_uids> includes_to;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> responses_to;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> conditions_to;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> milestones_to;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> conditions_from;
  std::map<uid_t, std::vector<uid_t>, cmp_uids> milestones_from;

  bool is_event_enabled(uid_t event_id);

  void set_event_executed(uid_t event_id);

  std::set<uid_t, cmp_uids> get_lock_set(uid_t event_id);

  std::set<uid_t, cmp_uids> get_inform_set(uid_t event_id);

  intermediate_dcr_worflow create_intermediate();

  dcr_workflow make_workflow(intermediate_dcr_worflow wf, char* name);
};
