#include "daemon.h"
#include "enclave_handle.h"

void main() {
  /*enclave_handle eh;
  dcr_workflow* wf = new dcr_workflow();
  dcr_event event;
  event.id = { 0,2 };
  wf->event_store[{0}] = event;
  std::map<uid_t, uid_t, cmp_uids>* peer_to_event_map = new std::map<uid_t, uid_t, cmp_uids>();
  uid_t id1 = { 0 };
  uid_t id2 = { (uint64_t)1, (uint64_t)2 };
  uid_t id3 = { (uint64_t)2, (uint64_t)2 };
  event.incoming_relations.conditions.push_back(id3);
  peer_to_event_map->insert_or_assign(id1, id2);
  peer_to_event_map->insert_or_assign(id2, id3);
  eh.init_enclave();
  uid_t peer_id = { 12,12 };
  uid_t event_id = { 0,1 };

  eh.e_provision_enclave(peer_id, wf, event_id, peer_to_event_map);
  delete peer_to_event_map;*/

	daemon sgxd(5556, 5555);
	//sgxd.start_daemon();
}
