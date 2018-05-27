#include "dcr.h"

bool dcr_workflow::is_event_enabled(uid_t event_id) {
  dcr_event event = event_store[event_id];
  if (event.excluded)
    return false;

  for each (uid_t condition_event in conditions_from[event.id]) {
    if (!event_store[condition_event].executed && !event_store[condition_event].excluded)
      return false;
  }

  for each (uid_t milestone_event in milestones_from[event.id]) {
    if (event_store[milestone_event].pending && !event_store[milestone_event].excluded)
      return false;
  }

  return true;
}

void dcr_workflow::set_event_executed(uid_t event_id) {
  dcr_event event = event_store[event_id];
  event.executed = true;
  event.pending = false;

  for each (uid_t exclude_event in excludes_to[event.id]) {
    event_store[exclude_event].excluded = true;
  }

  for each (uid_t include_event in includes_to[event.id]) {
    event_store[include_event].excluded = false;
  }

  for each (uid_t response_event in responses_to[event.id]) {
    event_store[response_event].pending = true;
  }
}

std::set<uid_t, cmp_uids> dcr_workflow::get_lock_set(uid_t event_id) {
  std::set<uid_t, cmp_uids> lock_set;
  std::vector<uid_t> excludes = excludes_to[event_id];
  std::vector<uid_t> includes = includes_to[event_id];
  std::vector<uid_t> responses = responses_to[event_id];
  lock_set.insert(excludes.begin(), excludes.end());
  lock_set.insert(includes.begin(), includes.end());
  lock_set.insert(responses.begin(), responses.end());

  std::set<uid_t, cmp_uids> constraints;
  for each (uid_t state_lock in lock_set) {
    std::vector<uid_t> conditions = conditions_to[state_lock];
    std::vector<uid_t> milestones = milestones_to[state_lock];
    constraints.insert(conditions.begin(), conditions.end());
    constraints.insert(milestones.begin(), milestones.end());
  }
  lock_set.insert(constraints.begin(), constraints.end());
  return lock_set;
}

std::set<uid_t, cmp_uids> dcr_workflow::get_inform_set(uid_t event_id) {
  std::set<uid_t, cmp_uids> inform_set;
  std::set<uid_t, cmp_uids> lock_set = get_lock_set(event_id);
  std::vector<uid_t> conditions = conditions_to[event_id];
  std::vector<uid_t> milestones = milestones_to[event_id];

  inform_set.insert(lock_set.begin(), lock_set.end());
  inform_set.insert(conditions.begin(), conditions.end());
  inform_set.insert(milestones.begin(), milestones.end());

  return inform_set;
}

intermediate_dcr_worflow dcr_workflow::create_intermediate() {
  intermediate_dcr_worflow temp;
  std::vector<uid_t> ids;
  std::vector<uid_t> excluded;
  std::vector<uid_t> pending;
  std::vector<uid_t> executed;
  std::vector<uid_t> flat_conditions_in;
  std::vector<uid_t> flat_conditions_out;
  std::vector<uid_t> flat_milestones_in;
  std::vector<uid_t> flat_milestones_out;
  std::vector<uid_t> flat_excludes_in;
  std::vector<uid_t> flat_excludes_out;
  std::vector<uid_t> flat_includes_in;
  std::vector<uid_t> flat_includes_out;
  std::vector<uid_t> flat_responses_in;
  std::vector<uid_t> flat_responses_out;

  for each (std::pair<uid_t, dcr_event> pair in event_store) {
    ids.push_back(pair.first);
    if (pair.second.excluded)
      excluded.push_back(pair.first);

    if (pair.second.pending)
      pending.push_back(pair.first);

    if (pair.second.executed)
      executed.push_back(pair.first);

    for each (uid_t condition_to in conditions_to[pair.first]) {
      flat_conditions_in.push_back(condition_to);
      flat_conditions_out.push_back(pair.first);
    }
    for each (uid_t milestone_to in milestones_to[pair.first]) {
      flat_milestones_in.push_back(milestone_to);
      flat_milestones_out.push_back(pair.first);
    }
    for each (uid_t exclude_to in excludes_to[pair.first]) {
      flat_excludes_in.push_back(exclude_to);
      flat_excludes_out.push_back(pair.first);
    }
    for each (uid_t include_to in includes_to[pair.first]) {
      flat_includes_in.push_back(include_to);
      flat_includes_out.push_back(pair.first);
    }
    for each (uid_t response_to in responses_to[pair.first]) {
      flat_responses_in.push_back(response_to);
      flat_responses_out.push_back(pair.first);
    }
  }
  temp.event_ids = &ids[0];
  temp.events_count = ids.size();
  temp.excluded = &excluded[0];
  temp.excluded_count = excluded.size();
  temp.pending = &pending[0];
  temp.pending_count = pending.size();
  temp.executed = &executed[0];
  temp.executed_count = executed.size();
  temp.conditions_count = flat_conditions_out.size();
  temp.dcr_conditions_in = &flat_conditions_in[0];
  temp.dcr_conditions_out = &flat_conditions_out[0];
  temp.milestones_count = flat_milestones_out.size();
  temp.dcr_milestones_in = &flat_milestones_in[0];
  temp.dcr_milestones_out = &flat_milestones_out[0];
  temp.excludes_count = flat_excludes_out.size();
  temp.dcr_excludes_in = &flat_excludes_in[0];
  temp.dcr_excludes_out = &flat_excludes_out[0];
  temp.includes_count = flat_includes_out.size();
  temp.dcr_includes_in = &flat_includes_in[0];
  temp.dcr_includes_out = &flat_includes_out[0];
  temp.responses_count = flat_responses_out.size();
  temp.dcr_responses_in = &flat_responses_in[0];
  temp.dcr_responses_out = &flat_responses_out[0];

  return temp;
}
