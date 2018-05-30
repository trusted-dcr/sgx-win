#include <iostream>
#include "stdafx.h"
#include "CppUnitTest.h"
#include "enclave_handle.h"
#include "dcr.cpp"
#include "test_objects.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#ifdef SGX_DEBUG
bool uids_equal(const uid_t& lhs, const uid_t& rhs) {
  return (lhs.id1 == rhs.id1) && (lhs.id2 == rhs.id2);
}
std::vector<command_req_t> command_reqs;
std::vector<command_rsp_t> command_rsps;

std::vector<append_req_t> append_reqs;
std::vector<append_rsp_t> append_rsps;

std::vector<poll_req_t> poll_reqs;
std::vector<poll_rsp_t> poll_rsps;

std::vector<election_req_t> election_reqs;
std::vector<election_rsp_t> election_rsps;

std::vector<log_req_t> log_reqs;
std::vector<log_rsp_t> log_rsps;

std::map<uid_t, std::vector<entry_t>, cmp_uids> event_to_log_map;

bool simulate = false;

void make_leader(enclave_handle* eh, uid_t target, std::vector<uid_t> quorum, uint32_t new_term) {
  for each (uid_t peer in quorum) {
    poll_rsp_t rsp = empty_poll_rsp(target, peer);
    poll_rsp_t rsp_maced = eh->e_test_set_mac_poll_rsp(rsp);
    eh->e_recv_poll_rsp(rsp_maced);
  }

  for each (uid_t peer in quorum) {
    election_rsp_t rsp2 = empty_election_rsp(target, peer);
    rsp2.term = new_term;
    rsp2.success = true;
    rsp2 = eh->e_test_set_mac_election_rsp(rsp2);
    eh->e_recv_election_rsp(rsp2);
  }
  Assert::IsTrue(eh->e_test_is_leader());

}

namespace enclave_tests
{
	TEST_CLASS(enclave_dcr_tests)	{
	public:
    enclave_handle eh;

    TEST_METHOD_INITIALIZE(setup) {
      eh.init_enclave();
    }

    TEST_METHOD_CLEANUP(teardown) {
      eh.destroy_enclave();
    }

		TEST_METHOD(simple_condition_wf)
		{
      dcr_workflow wf = wf_simple_condition();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
		}

    TEST_METHOD(simple_condition_wf_exec) {
      dcr_workflow wf = wf_simple_condition();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_milestone_wf) {
      dcr_workflow wf = wf_simple_milestone();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_milestone_wf_exec) {
      dcr_workflow wf = wf_simple_milestone();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_include_wf) {
      dcr_workflow wf = wf_simple_include();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_include_wf_exec) {
      dcr_workflow wf = wf_simple_include();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_exclude_wf) {
      dcr_workflow wf = wf_simple_exclude();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_exclude_wf_exec) {
      dcr_workflow wf = wf_simple_exclude();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_response_wf) {
      dcr_workflow wf = wf_simple_response();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_response_wf_exec) {
      dcr_workflow wf = wf_simple_response();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,2 }));
    }


    TEST_METHOD(DU_DE_wf_full_exec) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_configure_enclave({ 0 }, wf, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,5 }));

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));

      eh.e_test_execute({ 0,2 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));

      eh.e_test_execute({ 0,3 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,4 }));

      eh.e_test_execute({ 0,2 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_excluded(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,4 }));

      eh.e_test_execute({ 0,3 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,4 }));

      eh.e_test_execute({ 0,5 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,4 }));
    }
	};



  TEST_CLASS(test_peer_map_init) {
public:
    enclave_handle eh;

    TEST_METHOD_INITIALIZE(setup) {
      eh.init_enclave();
    }

    TEST_METHOD_CLEANUP(teardown) {
      eh.destroy_enclave();
    }

    TEST_METHOD(simple_peer_map_init) {
      dcr_workflow wf = wf_simple_condition();
      std::map<uid_t, uid_t, cmp_uids> peer_map = one_peer_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      uid_t leader = eh.e_test_leader_of_event({ 0,1 });
      uid_t event = eh.e_test_event_of_peer({ 0,1 });
      uint32_t cluster_size = eh.e_test_size_of_event_cluster();
      Assert::IsTrue(uids_equal({ 0,1 }, leader));
      Assert::IsTrue(uids_equal({ 0,1 }, event));
      Assert::AreEqual((uint32_t)1, cluster_size);
    }

    TEST_METHOD(two_peer_map_init) {
      dcr_workflow wf = wf_simple_condition();
      std::map<uid_t, uid_t, cmp_uids> peer_map = one_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      uid_t leader = eh.e_test_leader_of_event({ 0,1 });
      uid_t event = eh.e_test_event_of_peer({ 0,1 });
      uint32_t cluster_size = eh.e_test_size_of_event_cluster();
      Assert::IsTrue(uids_equal({ 0,2 }, leader));
      Assert::IsTrue(uids_equal({ 0,2 }, event));
      Assert::AreEqual((uint32_t)1, cluster_size);
    }

    TEST_METHOD(six_peer_map_init) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      uid_t leader1 = eh.e_test_leader_of_event({ 0,1 });
      uid_t leader2 = eh.e_test_leader_of_event({ 0,2 });
      uid_t leader3 = eh.e_test_leader_of_event({ 0,3 });
      uid_t event1 = eh.e_test_event_of_peer({ 0,1 });
      uid_t event2 = eh.e_test_event_of_peer({ 0,2 });
      uid_t event3 = eh.e_test_event_of_peer({ 0,3 });
      uid_t event4 = eh.e_test_event_of_peer({ 0,4 });
      uid_t event5 = eh.e_test_event_of_peer({ 0,5 });
      uid_t event6 = eh.e_test_event_of_peer({ 0,6 });
      uint32_t cluster_size = eh.e_test_size_of_event_cluster();
      Assert::IsTrue(uids_equal({ 0,1 }, leader1) || uids_equal({ 0,2 }, leader1));
      Assert::IsTrue(uids_equal({ 0,3 }, leader2) || uids_equal({ 0,4 }, leader2));
      Assert::IsTrue(uids_equal({ 0,5 }, leader3) || uids_equal({ 0,6 }, leader3));
      Assert::IsTrue(uids_equal({ 0,1 }, event1));
      Assert::IsTrue(uids_equal({ 0,1 }, event2));
      Assert::IsTrue(uids_equal({ 0,2 }, event3));
      Assert::IsTrue(uids_equal({ 0,2 }, event4));
      Assert::IsTrue(uids_equal({ 0,3 }, event5));
      Assert::IsTrue(uids_equal({ 0,3 }, event6));
      Assert::AreEqual((uint32_t)2, cluster_size);
    }
  };

  TEST_CLASS(response_test) {
public:
  enclave_handle eh;

    TEST_METHOD_INITIALIZE(setup) {
      eh.init_enclave();
    }

    TEST_METHOD_CLEANUP(teardown) {
      eh.destroy_enclave();
      command_reqs.clear();
      command_rsps.clear();
      append_reqs.clear();
      append_rsps.clear();
      poll_reqs.clear();
      poll_rsps.clear();
      election_reqs.clear();
      election_rsps.clear();
      event_to_log_map.clear();
    }

    TEST_METHOD(sanity_mac_test) {
      poll_req_t req1 = empty_poll_req({ 0,2 }, { 0,1 });
      poll_req_t req2 = empty_poll_req({ 0,2 }, { 0,1 });
      poll_req_t req1_maced = eh.e_test_set_mac_poll_req(req1);
      poll_req_t req2_maced = eh.e_test_set_mac_poll_req(req2);
      int cmp = memcmp(req1_maced.mac, req2_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);
      req2_maced = eh.e_test_set_mac_poll_req(req2_maced);
      cmp = memcmp(req1_maced.mac, req2_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);
      Assert::IsTrue(eh.e_test_verify_mac_poll_req(req1_maced));
      Assert::IsTrue(eh.e_test_verify_mac_poll_req(req2_maced));

      append_rsp_t rsp1 = empty_append_rsp(uid_t{ 0,2 }, uid_t{ 0,1 });
      append_rsp_t rsp2 = empty_append_rsp(uid_t{ 0,2 }, uid_t{ 0,1 });
      append_rsp_t rsp1_maced = eh.e_test_set_mac_append_rsp(rsp1);
      append_rsp_t rsp2_maced = eh.e_test_set_mac_append_rsp(rsp2);
      cmp = memcmp(rsp1_maced.mac, rsp2_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);
      rsp2_maced = eh.e_test_set_mac_append_rsp(rsp2_maced);
      cmp = memcmp(rsp1_maced.mac, rsp2_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);

      entry_t* entries = new entry_t[5];
      entries[0] = { 0 };
      entries[1] = { 1 };
      entries[2] = { 2 };
      entries[3] = { 3 };
      entries[4] = { 4 };

      append_req_t req3 = empty_append_req(uid_t{ 0,2 }, uid_t{ 0,1 });
      req3.entries = entries;
      req3.entries_n = 5;

      append_req_t req4 = empty_append_req(uid_t{ 0,2 }, uid_t{ 0,1 });
      req4.entries = entries;
      req4.entries_n = 5;

      append_req_t req3_maced = eh.e_test_set_mac_append_req(req3);
      append_req_t req4_maced = eh.e_test_set_mac_append_req(req4);
      cmp = memcmp(req1_maced.mac, req2_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);
      req4_maced = eh.e_test_set_mac_append_req(req4_maced);
      cmp = memcmp(req3_maced.mac, req4_maced.mac, SGX_CMAC_MAC_SIZE);
      Assert::IsTrue(cmp == 0);
    }


    TEST_METHOD(init_poll_test) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      Assert::AreEqual(0, (int)poll_reqs.size());

      std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // delta_max
      eh.e_set_time();
      Assert::AreEqual(1, (int)poll_reqs.size());

      poll_req_t req = poll_reqs[0];
      bool valid = eh.e_test_verify_mac_poll_req(req);
      Assert::IsTrue(valid);
    }

    TEST_METHOD(follower_successfull_poll_response) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      std::this_thread::sleep_for(std::chrono::milliseconds(8000)); // delta_max
      eh.e_set_time();
      Assert::AreEqual(1, (int)poll_reqs.size());
      poll_req_t req = poll_reqs[0];
      bool valid = eh.e_test_verify_mac_poll_req(req);
      Assert::IsTrue(valid);

      Assert::IsFalse(eh.e_test_is_leader());

      poll_rsp_t rsp = empty_poll_rsp({ 0,1 }, { 0,2 });
      poll_rsp_t rsp_maced = eh.e_test_set_mac_poll_rsp(rsp);
      eh.e_recv_poll_rsp(rsp_maced);
      Assert::IsTrue(election_reqs.size() == 1);

      Assert::IsFalse(eh.e_test_is_leader());

      election_rsp_t rsp2 = empty_election_rsp({ 0,1 }, { 0,2 });
      rsp2.term = 1;
      rsp2.success = true;
      rsp2 = eh.e_test_set_mac_election_rsp(rsp2);
      eh.e_recv_election_rsp(rsp2);

      Assert::IsTrue(eh.e_test_is_leader());
    }

    TEST_METHOD(leader_exec_is_executing_test) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);
      std::vector<uid_t> quorum;
      quorum.push_back({ 0,2 });
      make_leader(&eh, { 0,1 }, quorum, 1);

      Assert::IsTrue(append_reqs.size() == 1);

      command_req_t Exec_event_1 = {
        uid_t {0,1},
        uid_t {0,2},
        command_tag_t { uid_t {0,1}, EXEC },
        uid_t {0,1},
        {0}
      };
      command_req_t command_maced = eh.e_test_set_mac_command_req(Exec_event_1);

      Assert::IsFalse(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,5 }));

      eh.e_recv_command_req(command_maced);
      Assert::IsTrue(append_reqs.size() == 2);
      append_rsp_t rsp = empty_append_rsp({ 0,1 }, { 0,2 });
      rsp.term = append_reqs[1].term;
      rsp.prev_term = append_reqs[1].prev_term;
      rsp.prev_index = append_reqs[1].prev_index;
      rsp.success = true;
      rsp.last_index = 1;
      rsp = eh.e_test_set_mac_append_rsp(rsp);
      eh.e_recv_append_rsp(rsp);

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,3 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,4 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,5 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,4 }));
    }

    TEST_METHOD(leader_lock_msg_exec_sequence_test) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = ten_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);
      std::vector<uid_t> quorum;
      quorum.push_back({ 0,2 });
      make_leader(&eh, { 0,1 }, quorum, 1);

      Assert::IsTrue(append_reqs.size() == 1);

      command_req_t Lock_event_1 = {
        uid_t{ 0,1 },
        uid_t{ 0,2 },
        command_tag_t{ uid_t{ 0,1 }, LOCK },
        uid_t{ 0,1 },
        { 0 }
      };
      command_req_t command_maced = eh.e_test_set_mac_command_req(Lock_event_1);

      eh.e_recv_command_req(command_maced);

      Assert::IsTrue(append_reqs.size() == 2);

      //ack update, increment leaders commit index
      append_rsp_t rsp = empty_append_rsp({ 0, 1 }, { 0, 2 });
      rsp.term = append_reqs[1].term;
      rsp.prev_term = append_reqs[1].prev_term;
      rsp.prev_index = append_reqs[1].prev_index;
      rsp.success = true;
      rsp.last_index = 1;
      rsp = eh.e_test_set_mac_append_rsp(rsp);
      eh.e_recv_append_rsp(rsp);

      // leader has sent lock requests to the necessary leaders
      Assert::IsTrue(command_reqs.size() == 2);
      Assert::IsTrue(uids_equal(command_reqs[0].target, { 0, 7 }));
      Assert::IsTrue(uids_equal(command_reqs[1].target, { 0, 9 }));

      command_rsp_t rsp1 = empty_command_rsp({ 0, 1 }, { 0, 7 });
      command_rsp_t rsp2 = empty_command_rsp({ 0, 1 }, { 0, 9 });
      rsp1.tag = command_reqs[0].tag;
      rsp2.tag = command_reqs[1].tag;
      rsp1.success = true;
      rsp2.success = true;
      rsp1.leader = { 0, 7 };
      rsp2.leader = { 0, 9 };

      rsp1 = eh.e_test_set_mac_command_rsp(rsp1);
      rsp2 = eh.e_test_set_mac_command_rsp(rsp2);

      eh.e_recv_command_rsp(rsp1);
      Assert::IsTrue(command_reqs.size() == 2);
      Assert::IsTrue(append_reqs.size() == 2);
      eh.e_recv_command_rsp(rsp2);
      Assert::IsTrue(command_reqs.size() == 5);
      Assert::IsTrue(append_reqs.size() == 3);
      Assert::IsFalse(eh.e_test_is_executed({ 0,1 }));

      rsp = empty_append_rsp({ 0, 1 }, { 0, 2 });
      rsp.term = append_reqs[2].term;
      rsp.prev_term = append_reqs[2].prev_term;
      rsp.prev_index = append_reqs[2].prev_index;
      rsp.success = true;
      rsp.last_index = 2;
      rsp = eh.e_test_set_mac_append_rsp(rsp);
      eh.e_recv_append_rsp(rsp);
      Assert::IsTrue(eh.e_test_is_executed({ 0,1 }));
    }

    TEST_METHOD(double_enclave_handle_test) {
      enclave_handle eh1;
      enclave_handle eh2;
      eh1.init_enclave();
      eh2.init_enclave();
      dcr_workflow wf1 = wf_DU_DE();
      dcr_workflow wf2 = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map1 = ten_peers_two_peer_per_event_peer_map();
      std::map<uid_t, uid_t, cmp_uids> peer_map2 = ten_peers_two_peer_per_event_peer_map();
      eh1.e_configure_enclave({ 0,1 }, wf1, peer_map1);
      eh2.e_configure_enclave({ 0,1 }, wf2, peer_map1);
      std::vector<uid_t> quorum;
      quorum.push_back({ 0,2 });
      make_leader(&eh1, { 0,1 }, quorum, 1);
      make_leader(&eh2, { 0,1 }, quorum, 1);

      command_req_t Exec_event_1 = {
        uid_t{ 0,1 },
        uid_t{ 0,2 },
        command_tag_t{ uid_t{ 0,1 }, EXEC },
        uid_t{ 0,1 },
        { 0 }
      };
      command_req_t command_maced = eh1.e_test_set_mac_command_req(Exec_event_1);
      eh1.e_recv_command_req(command_maced);
      append_rsp_t rsp = empty_append_rsp({ 0,1 }, { 0,2 });
      rsp.term = append_reqs[2].term;
      rsp.prev_term = append_reqs[2].prev_term;
      rsp.prev_index = append_reqs[2].prev_index;
      rsp.success = true;
      rsp.last_index = 1;
      rsp = eh.e_test_set_mac_append_rsp(rsp);
      eh1.e_recv_append_rsp(rsp);
      Assert::IsTrue(eh1.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsFalse(eh2.e_test_is_executed(uid_t{ 0,1 }));
      eh1.destroy_enclave();
      eh2.destroy_enclave();
    }

    TEST_METHOD(follower_exec_test) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);

      append_req_t req1 = empty_append_req({ 0,1 }, { 0,2 });
      entry_t exec_entry = {
        1,
        1,
        {0,1}, //event
        {0,2}, //source
        {{1,1}, EXEC}
      };
      req1.entries = &exec_entry;
      req1.entries_n = 1;
      req1.commit_index = 0;
      req1.prev_index = 0;
      req1.prev_term = 0;
      req1.term = 1; //will make us ({0,2}) leader
      req1 = eh.e_test_set_mac_append_req(req1);
      eh.e_recv_append_req(req1);

      //not yet committed, so the follower should not have applied the execution yet.
      Assert::IsFalse(eh.e_test_is_executed({ 0,1 }));

      // commit the previous message
      req1 = empty_append_req({ 0,1 }, { 0,2 });
      req1.commit_index = 1;
      req1.prev_index = 1;
      req1.prev_term = 1;
      req1.term = 1;
      req1.entries = { 0 };
      req1.entries_n = 0;
      req1 = eh.e_test_set_mac_append_req(req1);
      eh.e_recv_append_req(req1);

      Assert::IsTrue(eh.e_test_is_executed({ 0,1 }));
    }

    TEST_METHOD(hearbeat_test) {
      dcr_workflow wf = wf_DU_DE();
      std::map<uid_t, uid_t, cmp_uids> peer_map = six_peers_two_peer_per_event_peer_map();
      eh.e_configure_enclave({ 0,1 }, wf, peer_map);
      std::vector<uid_t> quorum;
      quorum.push_back({ 0,2 });
      make_leader(&eh, { 0,1 }, quorum, 1);

      append_reqs.clear();
      election_reqs.clear();

      std::this_thread::sleep_for(std::chrono::milliseconds(600)); // delta_heartbeat
      eh.e_set_time();

      Assert::IsTrue(append_reqs.size() == 1);
    }

    TEST_METHOD(missing_resp_test) {
      leader_lock_msg_exec_sequence_test(); //leaves us in state with one missing response
      command_reqs.clear();
      append_reqs.clear();

      Assert::IsTrue(command_reqs.size() == 0);

      std::this_thread::sleep_for(std::chrono::milliseconds(600)); // delta_heartbeat = 500
      eh.e_set_time();

      Assert::IsTrue(command_reqs.size() == 3);
      Assert::IsTrue(uids_equal(command_reqs[0].target, { 0,3 }));
      Assert::IsTrue(append_reqs.size() == 1);

      std::this_thread::sleep_for(std::chrono::milliseconds(600)); // delta_heartbeat = 500
      eh.e_set_time();

      Assert::IsTrue(command_reqs.size() == 6);
      Assert::IsTrue(uids_equal(command_reqs[3].target, { 0,4 })); //test retry
      Assert::IsTrue(append_reqs.size() == 2);

      std::this_thread::sleep_for(std::chrono::milliseconds(600)); //delta_heartbeat = 500
      eh.e_set_time();

      Assert::IsTrue(command_reqs.size() == 9);
      Assert::IsTrue(uids_equal(command_reqs[6].target, { 0,3 })); //test retry
      Assert::IsTrue(append_reqs.size() == 3);
    }

    TEST_METHOD(get_log_map_size_test) {
      leader_lock_msg_exec_sequence_test(); //lots of executions
      command_reqs.clear();
      command_rsps.clear();
      append_reqs.clear();
      append_rsps.clear();
      poll_reqs.clear();
      poll_rsps.clear();
      election_reqs.clear();
      election_rsps.clear();
      log_reqs.clear();
      log_rsps.clear();

      eh.e_get_log();

      Assert::IsTrue(log_reqs.size() == 5);

      Assert::IsTrue(event_to_log_map.size() == 0);
      eh.e_recv_log_req(log_reqs[0]);
      Assert::IsTrue(log_rsps.size() == 1);
      entry_t* entries_new = (entry_t*) malloc(sizeof(entry_t) * 3);
      memcpy(entries_new, entries1(), sizeof(entry_t) * 3);
      log_rsp_t rsp = {
        uid_t {0,1},
        {0,1},
        true,
        uid_t {0,0},
        entries_new,
        3,
        {0}
      };

      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,3 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,5 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,7 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,9 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 5);

      for each (std::pair<uid_t, std::vector<entry_t>> event_to_log in event_to_log_map) {
        Assert::IsTrue(event_to_log.second.size() == 3);
      }
    }

    TEST_METHOD(get_log_map_size_variable_test) {
      leader_lock_msg_exec_sequence_test(); //lots of executions
      command_reqs.clear();
      command_rsps.clear();
      append_reqs.clear();
      append_rsps.clear();
      poll_reqs.clear();
      poll_rsps.clear();
      election_reqs.clear();
      election_rsps.clear();
      log_reqs.clear();
      log_rsps.clear();

      eh.e_get_log();

      Assert::IsTrue(log_reqs.size() == 5);

      Assert::IsTrue(event_to_log_map.size() == 0);
      eh.e_recv_log_req(log_reqs[0]);
      Assert::IsTrue(log_rsps.size() == 1);
      entry_t* entries_new = (entry_t*)malloc(sizeof(entry_t) * 4);
      memcpy(entries_new, entries1(), sizeof(entry_t) * 3);
      log_rsp_t rsp = {
        uid_t{ 0,1 },
        { 0,1 },
        true,
        uid_t{ 0,0 },
        entries_new,
        3,
        { 0 }
      };

      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      memcpy(entries_new, entries1(), sizeof(entry_t) * 4);
      entries_new[4] = entry_t{
        1,
        2,
        uid_t {0,4},
        uid_t{ 0,4 },
        command_tag_t {0,352, EXEC}
      };
      rsp.entries_n = 4;

      rsp.source = { 0,3 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,5 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,7 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 0);

      rsp.source = { 0,9 };
      rsp = eh.e_test_set_mac_log_rsp(rsp);
      eh.e_recv_log_rsp(rsp);
      Assert::IsTrue(event_to_log_map.size() == 5);

      for each (std::pair<uid_t, std::vector<entry_t>> event_to_log in event_to_log_map) {
        if (uids_equal(event_to_log.first, { 0,1 }))
          Assert::IsTrue(event_to_log.second.size() == 3);
        else
          Assert::IsTrue(event_to_log.second.size() == 4);
      }
    }
  };
}



void request_time() {

}

void send_command_req(command_req_t req) {
  command_reqs.push_back(req);
}

void send_command_rsp(command_rsp_t rsp) {
  command_rsps.push_back(rsp);
}

void send_append_req(append_req_t req) {
  throw - 1; //should never EVER be called
  append_reqs.push_back(req);
}

void send_append_req(append_req_t req, entry_t* entries, int size) {
  req.entries = entries;
  append_reqs.push_back(req);
}

void send_append_rsp(append_rsp_t rsp) {
  append_rsps.push_back(rsp);
}

void send_poll_req(poll_req_t req) {
  poll_reqs.push_back(req);
}

void send_poll_rsp(poll_rsp_t rsp) {
  poll_rsps.push_back(rsp);
}

void send_election_req(election_req_t req) {
  election_reqs.push_back(req);
}

void send_election_rsp(election_rsp_t rsp) {
  election_rsps.push_back(rsp);
}

void send_log_req(log_req_t req) {
  log_reqs.push_back(req);
}

void send_log_rsp(log_rsp_t rsp, entry_t* entries, int size) {
  rsp.entries = entries;
  log_rsps.push_back(rsp);
}

void return_logs(entry_t* flat_entry_list, uint32_t entry_length,
  uid_t* event_uids, uint32_t id_length,
  uint32_t* offset_list) {

  uint32_t acc = 0;
  uint32_t j = 0;
  for (uint32_t i = 0; i < id_length; i++) {
    for (j = 0; j < offset_list[i]; j++) {
      uid_t event_id = event_uids[i];
      entry_t entry = flat_entry_list[acc + j];
      if (event_to_log_map.find(event_id) == event_to_log_map.end())
        event_to_log_map[event_id] = std::vector<entry_t>();
      event_to_log_map[event_id].push_back(entry);
    }
    acc = acc + j;
  }
  for each (std::pair<uid_t, std::vector<entry_t>> event_to_log in event_to_log_map) {
    uid_t event = event_to_log.first;
    std::vector<entry_t> log = event_to_log.second;
  }
}

void print(const char* str) {

}

#endif // SGX_DEBUG