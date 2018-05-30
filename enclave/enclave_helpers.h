#pragma once
#include "dcr.h"
#include "raft.h"
#include "print_util.h"

std::map<uid_t, uid_t, cmp_uids> create_peer_to_event_map(uid_t* peers, uid_t* events, uint32_t map_size, uid_t own_id, uid_t* cluster_event) {
  std::map<uid_t, uid_t, cmp_uids> peer_to_event_map;
  for (uint32_t i = 0; i < map_size; i++) {
    if (uids_equal(peers[i], own_id))
      *cluster_event = events[i];
    peer_to_event_map[peers[i]] = events[i];
  }
  return peer_to_event_map;
}

std::map<uid_t, uid_t, cmp_uids> pick_leaders(std::map<uid_t, uid_t, cmp_uids> peer_to_event_map) {
  std::map<uid_t, uid_t, cmp_uids> leader_map;
  for each (std::pair<uid_t, uid_t> peer_event in peer_to_event_map) {
    if (leader_map.find(peer_event.second) == leader_map.end()) {
      leader_map[peer_event.second] = peer_event.first;
    }
  }
  return leader_map;
}

std::vector<uid_t> find_cluster_members(std::map<uid_t, uid_t, cmp_uids> peer_to_event_map, uid_t cluster_event) {
  std::vector<uid_t> members;
  for each (std::pair<uid_t, uid_t> peer_event in peer_to_event_map) {
    if (uids_equal(peer_event.second, cluster_event))
      members.push_back(peer_event.first);
  }
  return members;
}
