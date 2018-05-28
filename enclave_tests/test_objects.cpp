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

dcr_workflow wf_DU_DE() {
  dcr_workflow wf;
  wf.id = { 1,1 };
  wf.name = "DU_DE workflow";
  dcr_event propose_DU = simple_event({ 0,1 });
  dcr_event propose_DE = simple_event({ 0,2 });
  dcr_event accept_DU = simple_event({ 0,3 });
  dcr_event accept_DE = simple_event({ 0,4 });
  dcr_event hold_meeting = simple_event({ 0,5 });

  accept_DU.excluded = true;
  accept_DE.excluded = true;
  hold_meeting.pending = true;

  wf.event_store[propose_DU.id] = propose_DU;
  wf.event_store[propose_DE.id] = propose_DE;
  wf.event_store[accept_DU.id] = accept_DU;
  wf.event_store[accept_DE.id] = accept_DE;
  wf.event_store[hold_meeting.id] = hold_meeting;

  wf.conditions_to[propose_DU.id].push_back(propose_DE.id);
  wf.conditions_from[propose_DE.id].push_back(propose_DU.id);

  wf.milestones_to[accept_DU.id].push_back(hold_meeting.id);
  wf.milestones_from[hold_meeting.id].push_back(accept_DU.id);
  wf.milestones_to[accept_DE.id].push_back(hold_meeting.id);
  wf.milestones_from[hold_meeting.id].push_back(accept_DE.id);

  wf.includes_to[propose_DU.id].push_back(accept_DE.id);
  wf.includes_to[propose_DE.id].push_back(accept_DU.id);

  wf.excludes_to[accept_DU.id].push_back(accept_DE.id);
  wf.excludes_to[accept_DU.id].push_back(accept_DU.id);
  wf.excludes_to[accept_DE.id].push_back(accept_DU.id);
  wf.excludes_to[accept_DE.id].push_back(accept_DE.id);

  wf.responses_to[propose_DU.id].push_back(accept_DE.id);
  wf.responses_to[propose_DE.id].push_back(accept_DU.id);

  return wf;


}


//simple peer map:
std::map<uid_t, uid_t, cmp_uids> one_peer_peer_map() {
  std::map<uid_t, uid_t, cmp_uids> peer_map;
  peer_map[{0, 1}] = { 0,1 };
  return peer_map;
}

//two peer map
std::map<uid_t, uid_t, cmp_uids> one_peer_per_event_peer_map() {
  std::map<uid_t, uid_t, cmp_uids> peer_map;
  peer_map[{0, 1}] = { 0,2 };
  peer_map[{0, 2}] = { 0,1 };
  return peer_map;
}

//six peer map on two event
std::map<uid_t, uid_t, cmp_uids> six_peers_two_peer_per_event_peer_map() {
  std::map<uid_t, uid_t, cmp_uids> peer_map;
  peer_map[{0, 1}] = { 0,1 };
  peer_map[{0, 2}] = { 0,1 };
  peer_map[{0, 3}] = { 0,2 };
  peer_map[{0, 4}] = { 0,2 };
  peer_map[{0, 5}] = { 0,3 };
  peer_map[{0, 6}] = { 0,3 };
  return peer_map;
}

command_req_t empty_command_req(uid_t target, uid_t source) {
  return command_req_t();
}

command_rsp_t empty_command_rsp(uid_t target, uid_t source) {
  return command_rsp_t();
}

append_req_t empty_append_req(uid_t target, uid_t source) {
  append_req_t req = {
    uid_t{ 0,2 },
    uid_t{ 0,1 },
    1,
    0,
    0,
    0,
    NULL,
    0,
    { 0 }
  };

  return req;
}

append_rsp_t empty_append_rsp(uid_t target, uid_t source) {
  append_rsp_t rsp1 = {
    target,
    source,
    0,
    0,
    0,
    true,
    0,
    { 0 }
  };
  return rsp1;
}

election_req_t empty_election_req(uid_t target, uid_t source) {
  return election_req_t();
}

election_rsp_t empty_election_rsp(uid_t target, uid_t source) {
  return election_rsp_t();
}

log_req_t empty_log_req(uid_t target, uid_t source) {
  return log_req_t();
}

log_rsp_t empty_log_rsp(uid_t target, uid_t source) {
  return log_rsp_t();
}

poll_req_t empty_poll_req(uid_t target, uid_t source) {
  poll_req_t req1 = {
    target,
    source,
    0,
    0,
    0,
    { 0 }
  };

  return req1;
}

poll_rsp_t empty_poll_rsp(uid_t target, uid_t source) {
  poll_rsp_t rsp = {
    target,
    source,
    0,
    true,
    {0}
  };
  return rsp;
}
