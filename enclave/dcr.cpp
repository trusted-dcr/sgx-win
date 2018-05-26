#include "dcr.h"

bool dcr_workflow::is_event_enabled(uid_t event_id) {
  dcr_event _event = event_store[event_id];
  if (_event.excluded)
    return false;

  for (unsigned int i = 0; i < _event.incoming_relations.conditions.size(); i++) {
    dcr_event condition_event = event_store[_event.incoming_relations.conditions[i]];
    if (!condition_event.executed && !condition_event.excluded)
      return false;
  }

  for (unsigned int i = 0; i < _event.incoming_relations.milestones.size(); i++) {
    if (!event_store[_event.incoming_relations.milestones[i]].pending)
      return false;
  }

  return true;
}

void dcr_workflow::set_event_executed(uid_t event_id) {
  dcr_event _event = event_store[event_id];
  _event.executed = true;
  _event.pending = false;

  for (unsigned int i = 0; i < _event.outgoing_relations.excludes.size(); i++) {
    event_store[_event.outgoing_relations.excludes[i]].excluded = true;
  }

  for (unsigned int i = 0; i < _event.outgoing_relations.includes.size(); i++) {
    event_store[_event.outgoing_relations.includes[i]].excluded = false;
  }

  for (unsigned int i = 0; i < _event.outgoing_relations.responses.size(); i++) {
    event_store[_event.outgoing_relations.responses[i]].pending = true;
  }
}

std::set<uid_t, cmp_uids> dcr_workflow::get_lock_set(uid_t event_id) {
  std::set<uid_t, cmp_uids> lock_set;
  std::vector<uid_t> excludes = event_store[event_id].outgoing_relations.excludes;
  std::vector<uid_t> includes = event_store[event_id].outgoing_relations.includes;
  std::vector<uid_t> responses = event_store[event_id].outgoing_relations.responses;
  lock_set.insert(excludes.begin(), excludes.end());
  lock_set.insert(includes.begin(), includes.end());
  lock_set.insert(responses.begin(), responses.end());

  std::set<uid_t, cmp_uids> constraints;
  for each (uid_t state_lock in lock_set) {
    std::vector<uid_t> conditions = event_store[state_lock].outgoing_relations.conditions;
    std::vector<uid_t> milestones = event_store[state_lock].outgoing_relations.milestones;
    constraints.insert(conditions.begin(), conditions.end());
    constraints.insert(milestones.begin(), milestones.end());
  }
  lock_set.insert(constraints.begin(), constraints.end());
  return lock_set;
}

std::set<uid_t, cmp_uids> dcr_workflow::get_inform_set(uid_t event_id) {
  std::set<uid_t, cmp_uids> inform_set;
  std::set<uid_t, cmp_uids> lock_set = get_lock_set(event_id);
  std::vector<uid_t> conditions = event_store[event_id].outgoing_relations.conditions;
  std::vector<uid_t> milestones = event_store[event_id].outgoing_relations.milestones;

  inform_set.insert(lock_set.begin(), lock_set.end());
  inform_set.insert(conditions.begin(), conditions.end());
  inform_set.insert(milestones.begin(), milestones.end());

  return inform_set;
}

//void enclave_create_event(int event_size, enclave_dcr_event* _event_in,
//	int in_excludes_size, uid_t* in_excludes,
//	int in_includes_size, uid_t* in_includes,
//	int in_responses_size, uid_t* in_responses,
//	int in_conditions_size, uid_t* in_conditions,
//	int in_milestones_size, uid_t* in_milestones,
//	int out_excludes_size, uid_t* out_excludes,
//	int out_includes_size, uid_t* out_includes,
//	int out_responses_size, uid_t* out_responses,
//	int out_conditions_size, uid_t* out_conditions,
//	int out_milestones_size, uid_t* out_milestones,
//	int name_size, char* name)
//{
//	// Deref event pointer to enable persistance in store
//	enclave_dcr_event _event = *_event_in;
//
//	// Copy in buffer's data to enclave
//	copy_relation_sets(_event.incoming_relations, in_excludes, in_includes,
//		in_responses, in_conditions, in_milestones);
//	copy_relation_sets(_event.outgoing_relations, out_excludes, out_includes,
//		out_responses, out_conditions, out_milestones);
//
//	_event.name_size = name_size;
//	_event.name = new char[name_size];
//	memcpy(_event.name, name, name_size);
//
//	// Store event:
//	event_store[_event.id] = _event;
//}
