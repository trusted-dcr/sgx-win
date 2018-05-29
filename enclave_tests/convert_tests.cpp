#include "stdafx.h"
#include <CppUnitTest.h>

#include "protos\crypto.pb.cc"
#include "protos\dcr.pb.cc"
#include "protos\network.pb.cc"
#include "protos\raft.pb.cc"
#include "protos\sgxd.pb.cc"
#include "convert.cpp"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
TEST_CLASS(convert_tests) {
private:
	bool equals(uid_t lhs, uid_t rhs) {
		return lhs.id1 == rhs.id1
			&& lhs.id2 == rhs.id2;
	}

	bool equals(sgx_cmac_128bit_tag_t lhs, sgx_cmac_128bit_tag_t rhs) {
		return lhs[0] == rhs[0]
			&& lhs[1] == rhs[1]
			&& lhs[2] == rhs[2]
			&& lhs[3] == rhs[3]
			&& lhs[4] == rhs[4]
			&& lhs[5] == rhs[5]
			&& lhs[6] == rhs[6]
			&& lhs[7] == rhs[7]
			&& lhs[8] == rhs[8]
			&& lhs[9] == rhs[9]
			&& lhs[10] == rhs[10]
			&& lhs[11] == rhs[11]
			&& lhs[12] == rhs[12]
			&& lhs[13] == rhs[13]
			&& lhs[14] == rhs[14]
			&& lhs[15] == rhs[15];
	}

	bool equals(command_tag_t lhs, command_tag_t rhs) {
		return lhs.type == rhs.type
			&& equals(lhs.uid, rhs.uid);
	}

	bool equals(entry_t lhs, entry_t rhs) {
		return lhs.index == rhs.index
			&& lhs.term == rhs.term
			&& equals(lhs.event, rhs.event)
			&& equals(lhs.source, rhs.source)
			&& equals(lhs.tag, rhs.tag);
	}

	bool equals(entry_t* lhs_entries, uint32_t lhs_entries_n, entry_t* rhs_entries, uint32_t rhs_entries_n) {
		if (lhs_entries_n != rhs_entries_n)
			return false;

		for (uint32_t i = 0; i < lhs_entries_n; i++)
			if (!equals(lhs_entries[i], rhs_entries[i]))
				return false;

		return true;
	}

	bool equals(command_req_t lhs, command_req_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& equals(lhs.tag, rhs.tag)
			&& equals(lhs.event, rhs.event)
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(command_rsp_t lhs, command_rsp_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& equals(lhs.tag, rhs.tag)
			&& lhs.success == rhs.success
			&& equals(lhs.leader, rhs.leader)
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(append_req_t lhs, append_req_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.prev_term == rhs.prev_term
			&& lhs.prev_index == rhs.prev_index
			&& lhs.commit_index == rhs.commit_index
			&& equals(lhs.entries, lhs.entries_n, rhs.entries, rhs.entries_n)
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(append_rsp_t lhs, append_rsp_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.prev_term == rhs.prev_term
			&& lhs.prev_index == rhs.prev_index
			&& lhs.success == rhs.success
			&& lhs.last_index == rhs.last_index
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(poll_req_t lhs, poll_req_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.last_term == rhs.last_term
			&& lhs.last_index == rhs.last_index
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(poll_rsp_t lhs, poll_rsp_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.success == rhs.success
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(election_req_t lhs, election_req_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.last_term == rhs.last_term
			&& lhs.last_index == rhs.last_index
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(election_rsp_t lhs, election_rsp_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.term == rhs.term
			&& lhs.success == rhs.success
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(log_req_t lhs, log_req_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& equals(lhs.mac, rhs.mac);
	}

	bool equals(log_rsp_t lhs, log_rsp_t rhs) {
		return equals(lhs.target, rhs.target)
			&& equals(lhs.source, rhs.source)
			&& lhs.success == rhs.success
			&& equals(lhs.leader, rhs.leader)
			&& equals(lhs.entries, lhs.entries_n, rhs.entries, rhs.entries_n)
			&& equals(lhs.mac, rhs.mac);
	}

public:
	TEST_METHOD(test_uid) {
		uid_t before { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e };
		uid_t after = convert::from_wire(convert::to_wire(before));
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_mac) {
		sgx_cmac_128bit_tag_t before = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 };
		sgx_cmac_128bit_tag_t after;
		convert::from_wire(convert::to_wire(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_command_tag) {
		command_tag_t before {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			command_type_t::CHECKPOINT
		};

		command_tag_t after = convert::from_wire(convert::to_wire(before));
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_entry) {
		entry_t before {
			0xd5b777ed,
			0x9bd35026,
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			command_tag_t { uid_t { 0x5fefd12b45e607c4, 0xc73363e8a66bd5d8 }, command_type_t::EXEC }
		};

		entry_t after = convert::from_wire(convert::to_wire(before));
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_command_req) {
		command_req_t before {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			command_tag_t { uid_t { 0x5fefd12b45e607c4, 0xc73363e8a66bd5d8 }, command_type_t::EXEC },
			uid_t { 0x01824e684b45a9cd, 0x954ce707270a313a },
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		command_req_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_command_rsp) {
		command_rsp_t before {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			command_tag_t { uid_t { 0x5fefd12b45e607c4, 0xc73363e8a66bd5d8 }, command_type_t::EXEC },
			true,
			uid_t { 0x01824e684b45a9cd, 0x954ce707270a313a },
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		command_rsp_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_append_req) {
		entry_t entries[] = {
			entry_t {
				0xd5b777ed,
				0x9bd35026,
				uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
				uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
				command_tag_t { uid_t { 0x5fefd12b45e607c4, 0xc73363e8a66bd5d8 }, command_type_t::EXEC }
			},

			entry_t {
				0x246eb14b,
				0x7308fdc1,
				uid_t { 0xd3df03b62d74bca9, 0xfc16cf215c29c267 },
				uid_t { 0xc35f5b15f4aa89ca, 0xfe7e3a3f9c360b2a },
				command_tag_t { uid_t { 0xaab6417587d6e177, 0xacba6e67b7637a7b }, command_type_t::ABORT }
			},

			entry_t {
				0x42462156,
				0xd12818eb,
				uid_t { 0xe632f4bcbdbb9724, 0x27b1cbe128eb3f76 },
				uid_t { 0x6121737128398933, 0xb4acba90a71621ed },
				command_tag_t { uid_t { 0xc641e55ec2044a55, 0x423486e67a15b86f }, command_type_t::CHECKPOINT }
			}
		};

		append_req_t before {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			0x4b45a9cd,
			0x954ce707,
			0x270a313a,
			entries,
			3,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		append_req_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_append_rsp) {
		append_rsp_t before {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			0x4b45a9cd,
			0x954ce707,
			true,
			0x270a313a,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		append_rsp_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_poll_req) {
		poll_req_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			0x4b45a9cd,
			0x954ce707,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		poll_req_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_poll_rsp) {
		poll_rsp_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			true,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		poll_rsp_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_election_req) {
		election_req_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			0x4b45a9cd,
			0x954ce707,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		election_req_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_election_rsp) {
		election_rsp_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			0x01824e68,
			true,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		election_rsp_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_log_req) {
		log_req_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		log_req_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}

	TEST_METHOD(test_log_rsp) {
		entry_t entries[] = {
			entry_t {
				0xd5b777ed,
				0x9bd35026,
				uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
				uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
				command_tag_t { uid_t { 0x5fefd12b45e607c4, 0xc73363e8a66bd5d8 }, command_type_t::EXEC }
			},

			entry_t {
				0x246eb14b,
				0x7308fdc1,
				uid_t { 0xd3df03b62d74bca9, 0xfc16cf215c29c267 },
				uid_t { 0xc35f5b15f4aa89ca, 0xfe7e3a3f9c360b2a },
				command_tag_t { uid_t { 0xaab6417587d6e177, 0xacba6e67b7637a7b }, command_type_t::ABORT }
			},

			entry_t {
				0x42462156,
				0xd12818eb,
				uid_t { 0xe632f4bcbdbb9724, 0x27b1cbe128eb3f76 },
				uid_t { 0x6121737128398933, 0xb4acba90a71621ed },
				command_tag_t { uid_t { 0xc641e55ec2044a55, 0x423486e67a15b86f }, command_type_t::CHECKPOINT }
			}
		};

		log_rsp_t before = {
			uid_t { 0x5237a3bb118757dc, 0x3af75176ba6d8d6e },
			uid_t { 0x097832fcfc06079d, 0xd5544df3e07203c6 },
			true,
			uid_t { 0x01824e684b45a9cd, 0x954ce707270a313a },
			entries,
			3,
			{ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16 }
		};

		log_rsp_t after;
		convert::unpack(convert::pack(before), after);
		Assert::IsTrue(equals(before, after));
	}
};
