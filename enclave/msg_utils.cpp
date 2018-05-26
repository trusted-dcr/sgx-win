#include "msg.h"
#include "msg_util.h"

bool uids_equal(const uid_t& lhs, const uid_t& rhs) {
  return (lhs.id1 == rhs.id1) && (lhs.id2 == rhs.id2);
}

bool tags_equal(const command_tag_t& lhs, const command_tag_t& rhs) {
  return uids_equal(lhs.uid, rhs.uid) && lhs.type == rhs.type;
}

bool entry_equal(const entry_t& lhs, const entry_t& rhs) {
  return uids_equal(lhs.event, rhs.event) && uids_equal(lhs.source, rhs.source) && tags_equal(lhs.tag, rhs.tag) && lhs.term == rhs.term;
}

uid_t generate_random_uid() {
  unsigned char* rand1 = new unsigned char[8];
  unsigned char* rand2 = new unsigned char[8];
  uid_t id;
  sgx_status_t status;
  do {
    status = sgx_read_rand(rand1, 8);
    status = sgx_read_rand(rand2, 8);
    id.id1 = (uint64_t)rand1;
    id.id2 = (uint64_t)rand2;
  } while (status != SGX_SUCCESS);
  return id;
}