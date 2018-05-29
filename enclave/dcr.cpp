#include "dcr.h"

bool dcr_workflow::is_event_enabled(uid_t event_id) {
  dcr_event event = event_store[event_id];
  if (event.excluded)
    return false;

  auto a = conditions_from[event.id];
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
  event_store[event_id].executed = true;
  event_store[event_id].pending = false;

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
  for each (uid_t constraint in constraints) {
    lock_set.emplace(constraint);
  }
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
  intermediate_dcr_worflow ret;
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
  if (ids.size() > 0) {
    ret.event_ids = new uid_t[ids.size()];
    std::copy(ids.begin(), ids.end(), ret.event_ids);
  }
  ret.events_count = ids.size();

  if (excluded.size() > 0) {
    ret.excluded = new uid_t[excluded.size()];
    std::copy(excluded.begin(), excluded.end(), ret.excluded);
  }
  ret.excluded_count = excluded.size();
  if (pending.size() > 0) {
    ret.pending = new uid_t[pending.size()];
    std::copy(pending.begin(), pending.end(), ret.pending);
  }
  ret.pending_count = pending.size();

  if (executed.size() > 0) {
    ret.executed = new uid_t[executed.size()];
    std::copy(executed.begin(), executed.end(), ret.executed);
  }
  ret.executed_count = executed.size();

  ret.conditions_count = flat_conditions_out.size();
  if (flat_conditions_in.size() > 0) {
    ret.dcr_conditions_in = new uid_t[flat_conditions_in.size()];
    std::copy(flat_conditions_in.begin(), flat_conditions_in.end(), ret.dcr_conditions_in);
  }
  if (flat_conditions_out.size() > 0) {
    ret.dcr_conditions_out = new uid_t[flat_conditions_out.size()];
    std::copy(flat_conditions_out.begin(), flat_conditions_out.end(), ret.dcr_conditions_out);
  }

  ret.milestones_count = flat_milestones_out.size();
  if (flat_milestones_in.size() > 0) {
    ret.dcr_milestones_in = new uid_t[flat_milestones_in.size()];
    std::copy(flat_milestones_in.begin(), flat_milestones_in.end(), ret.dcr_milestones_in);
  }
  if (flat_milestones_out.size() > 0) {
    ret.dcr_milestones_out = new uid_t[flat_milestones_out.size()];
    std::copy(flat_milestones_out.begin(), flat_milestones_out.end(), ret.dcr_milestones_out);
  }

  ret.excludes_count = flat_excludes_out.size();
  if (flat_excludes_in.size() > 0) {
    ret.dcr_excludes_in = new uid_t[flat_excludes_in.size()];
    std::copy(flat_excludes_in.begin(), flat_excludes_in.end(), ret.dcr_excludes_in);
  }
  if (flat_excludes_out.size() > 0) {
    ret.dcr_excludes_out = new uid_t[flat_excludes_out.size()];
    std::copy(flat_excludes_out.begin(), flat_excludes_out.end(), ret.dcr_excludes_out);
  }

  ret.includes_count = flat_includes_out.size();
  if (flat_includes_in.size() > 0) {
    ret.dcr_includes_in = new uid_t[flat_includes_in.size()];
    std::copy(flat_includes_in.begin(), flat_includes_in.end(), ret.dcr_includes_in);
  }
  if (flat_includes_out.size() > 0) {
    ret.dcr_includes_out = new uid_t[flat_includes_out.size()];
    std::copy(flat_includes_out.begin(), flat_includes_out.end(), ret.dcr_includes_out);
  }

  ret.responses_count = flat_responses_out.size();
  if (flat_responses_in.size() > 0) {
    ret.dcr_responses_in = new uid_t[flat_responses_in.size()];
    std::copy(flat_responses_in.begin(), flat_responses_in.end(), ret.dcr_responses_in);
  }
  if (flat_responses_out.size() > 0) {
    ret.dcr_responses_out = new uid_t[flat_responses_out.size()];
    std::copy(flat_responses_out.begin(), flat_responses_out.end(), ret.dcr_responses_out);
  }

  return ret;
}

dcr_workflow dcr_workflow::make_workflow(intermediate_dcr_worflow wf) {
	dcr_workflow new_wf;

  //set ids
  for (uint32_t i = 0; i < wf.events_count; i++) {
    dcr_event event = { 0 };
    event.id = wf.event_ids[i];
    new_wf.event_store[event.id] = event;
  }

  //set event state
  for (uint32_t i = 0; i < wf.excluded_count; i++) {
    new_wf.event_store[wf.excluded[i]].excluded = true;
  }
  for (uint32_t i = 0; i < wf.pending_count; i++) {
    new_wf.event_store[wf.pending[i]].pending = true;
  }
  for (uint32_t i = 0; i < wf.executed_count; i++) {
    new_wf.event_store[wf.executed[i]].executed = true;
  }

  // set relations
  for (int i = 0; i < wf.conditions_count; i++) {
    new_wf.conditions_to[wf.dcr_conditions_out[i]].push_back(wf.dcr_conditions_in[i]);
    new_wf.conditions_from[wf.dcr_conditions_in[i]].push_back(wf.dcr_conditions_out[i]);
  }
  for (int i = 0; i < wf.milestones_count; i++) {
    new_wf.milestones_to[wf.dcr_milestones_out[i]].push_back(wf.dcr_milestones_in[i]);
    new_wf.milestones_from[wf.dcr_milestones_in[i]].push_back(wf.dcr_milestones_out[i]);
  }
  for (int i = 0; i < wf.excludes_count; i++) {
    new_wf.excludes_to[wf.dcr_excludes_out[i]].push_back(wf.dcr_excludes_in[i]);
  }
  for (int i = 0; i < wf.includes_count; i++) {
    new_wf.includes_to[wf.dcr_includes_out[i]].push_back(wf.dcr_includes_in[i]);
  }
  for (int i = 0; i < wf.responses_count; i++) {
    new_wf.responses_to[wf.dcr_responses_out[i]].push_back(wf.dcr_responses_in[i]);
  }

  return new_wf;
}
