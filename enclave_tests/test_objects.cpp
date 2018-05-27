#include "stdafx.h"
#include "test_objects.h"
 

//workflows:

dcr_event simple_event(uid_t id) {
  dcr_event e;
  e.id = id;
  e.excluded = false;
  e.executed = false;
  e.pending = false;
  return e;
}

dcr_workflow wf_simple_condition() {
   dcr_workflow wf;
   wf.id = { 1,1 };
   wf.name = "simple condition workflow";
   dcr_event e1 = simple_event({ 0,1 });
   dcr_event e2 = simple_event({ 0,2 });
   wf.event_store[e1.id] = e1;
   wf.event_store[e2.id] = e2;
   wf.conditions_to[e1.id].push_back(e2.id);
   wf.conditions_from[e2.id].push_back(e1.id);
   return wf;
 }

dcr_workflow wf_simple_milestone() {
  dcr_workflow wf;
  wf.id = { 1,1 };
  wf.name = "simple milestone workflow";
  dcr_event e1 = simple_event({ 0,1 });
  dcr_event e2 = simple_event({ 0,2 });
  e1.pending = true;
  wf.event_store[e1.id] = e1;
  wf.event_store[e2.id] = e2;
  wf.milestones_to[e1.id].push_back(e2.id);
  wf.milestones_from[e2.id].push_back(e1.id);
  return wf;
}

dcr_workflow wf_simple_include() {
  dcr_workflow wf;
  wf.id = { 1,1 };
  wf.name = "simple include workflow";
  dcr_event e1 = simple_event({ 0,1 });
  dcr_event e2 = simple_event({ 0,2 });
  e2.excluded = true;
  wf.event_store[e1.id] = e1;
  wf.event_store[e2.id] = e2;
  wf.includes_to[e1.id].push_back(e2.id);
  return wf;
}

dcr_workflow wf_simple_exclude() {
  dcr_workflow wf;
  wf.id = { 1,1 };
  wf.name = "simple exclude workflow";
  dcr_event e1 = simple_event({ 0,1 });
  dcr_event e2 = simple_event({ 0,2 });
  wf.event_store[e1.id] = e1;
  wf.event_store[e2.id] = e2;
  wf.excludes_to[e1.id].push_back(e2.id);
  return wf;
}

dcr_workflow wf_simple_response() {
  dcr_workflow wf;
  wf.id = { 1,1 };
  wf.name = "simple response workflow";
  dcr_event e1 = simple_event({ 0,1 });
  dcr_event e2 = simple_event({ 0,2 });
  wf.event_store[e1.id] = e1;
  wf.event_store[e2.id] = e2;
  wf.responses_to[e1.id].push_back(e2.id);
  return wf;
}


//simple peer map:
std::map<uid_t, uid_t, cmp_uids> one_peer_peer_map() {
  std::map<uid_t, uid_t, cmp_uids> peer_map;
  peer_map[{0, 1}] = { 0,1 };
  return peer_map;
}
