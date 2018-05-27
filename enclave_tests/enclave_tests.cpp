#include <iostream>
#include "stdafx.h"
#include "CppUnitTest.h"
#include "enclave_handle.h"
#include "dcr.cpp"
#include "test_objects.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

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
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
		}

    TEST_METHOD(simple_condition_wf_exec) {
      dcr_workflow wf = wf_simple_condition();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_milestone_wf) {
      dcr_workflow wf = wf_simple_milestone();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_milestone_wf_exec) {
      dcr_workflow wf = wf_simple_milestone();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_include_wf) {
      dcr_workflow wf = wf_simple_include();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_include_wf_exec) {
      dcr_workflow wf = wf_simple_include();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,2 }));
      Assert::IsFalse(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_exclude_wf) {
      dcr_workflow wf = wf_simple_exclude();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_exclude_wf_exec) {
      dcr_workflow wf = wf_simple_exclude();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_excluded(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_response_wf) {
      dcr_workflow wf = wf_simple_response();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      Assert::IsTrue(eh.e_test_is_enabled(uid_t{ 0,1 }));
      Assert::IsFalse(eh.e_test_is_pending(uid_t{ 0,2 }));
    }

    TEST_METHOD(simple_response_wf_exec) {
      dcr_workflow wf = wf_simple_response();
      std::map<uid_t, uid_t, cmp_uids> peer_map;
      eh.e_provision_enclave({ 0 }, wf, { 0,1 }, peer_map);

      eh.e_test_execute({ 0,1 });

      Assert::IsTrue(eh.e_test_is_executed(uid_t{ 0,1 }));
      Assert::IsTrue(eh.e_test_is_pending(uid_t{ 0,2 }));
    }
	};



  TEST_CLASS(test_response_test) {
public:
    enclave_handle eh;
    std::vector<command_req_t> command_reqs;
    std::vector<command_rsp_t> command_rsps;

    std::vector<append_req_t> append_reqs;
    std::vector<append_rsp_t> append_rsps;

    std::vector<poll_req_t> poll_reqs;
    std::vector<poll_rsp_t> poll_rsps;

    std::vector<election_req_t> election_reqs;
    std::vector<election_rsp_t> election_rsps;

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
    }

  };
}

void request_time() {

}

void send_command_req(command_req_t req) {

}

void send_command_rsp(command_rsp_t rsp) {

}

void send_append_req(append_req_t req) {

}

void send_append_req(append_req_t req, entry_t* entries, int size) {

}

void send_append_rsp(append_rsp_t rsp) {

}

void send_poll_req(poll_req_t req) {

}

void send_poll_rsp(poll_rsp_t rsp) {

}

void send_election_req(election_req_t req) {

}

void send_election_rsp(election_rsp_t rsp) {

}
