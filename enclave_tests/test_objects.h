#pragma once
#include "dcr.h"

dcr_event simple_event(uid_t id);

dcr_workflow wf_simple_condition();
dcr_workflow wf_simple_milestone();
dcr_workflow wf_simple_include();
dcr_workflow wf_simple_exclude();
dcr_workflow wf_simple_response();
dcr_workflow wf_DU_DE();

std::map<uid_t, uid_t, cmp_uids> one_peer_peer_map();
std::map<uid_t, uid_t, cmp_uids> one_peer_per_event_peer_map();
std::map<uid_t, uid_t, cmp_uids> six_peers_two_peer_per_event_peer_map();
std::map<uid_t, uid_t, cmp_uids> ten_peers_two_peer_per_event_peer_map();

command_req_t empty_command_req(uid_t target, uid_t source);
command_rsp_t empty_command_rsp(uid_t target, uid_t source);

append_req_t empty_append_req(uid_t target, uid_t source);
append_rsp_t empty_append_rsp(uid_t target, uid_t source);

election_req_t empty_election_req(uid_t target, uid_t source);
election_rsp_t empty_election_rsp(uid_t target, uid_t source);

log_req_t empty_log_req(uid_t target, uid_t source);
log_rsp_t empty_log_rsp(uid_t target, uid_t source);

poll_req_t empty_poll_req(uid_t target, uid_t source);
poll_rsp_t empty_poll_rsp(uid_t target, uid_t source);

