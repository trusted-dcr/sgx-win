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


