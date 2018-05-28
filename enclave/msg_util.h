#pragma once
#include <sgx_trts.h>
#include "msg.h"


struct cmp_uids {
  bool operator()(const uid_t& lhs, const uid_t& rhs) const {
    if (lhs.id1 == rhs.id1)
      return lhs.id2 < rhs.id2;

    return lhs.id1 < rhs.id1;
  }
};

bool uids_equal(const uid_t& lhs, const uid_t& rhs);

bool tags_equal(const command_tag_t& lhs, const command_tag_t& rhs);

bool entry_equal(const entry_t& lhs, const entry_t& rhs);

uid_t generate_random_uid(); 

entry_t get_empty_entry();