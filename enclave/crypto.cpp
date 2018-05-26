#include "crypto.h"

bool cmp_macs(sgx_cmac_128bit_tag_t mac_1, sgx_cmac_128bit_tag_t mac_2) {
  for (int i = 0; i < SGX_CMAC_MAC_SIZE; i++) {
    if (mac_1[i] != mac_2[i])
      return false;
  }
  return true;
}

//append_req_t is nested
sgx_status_t get_mac_append_req(append_req_t* append_req, sgx_cmac_128bit_tag_t* tag) {
  append_req_t cpy;
  cpy.commit_index = append_req->commit_index;
  cpy.entries_n = append_req->entries_n;
  cpy.prev_index = append_req->prev_index;
  cpy.prev_term = append_req->prev_term;
  cpy.source = append_req->source;
  cpy.target = append_req->target;
  cpy.term = append_req->term;
  cpy.entries = 0;

  sgx_cmac_128bit_tag_t tags[2];
  sgx_status_t ret = get_mac_flattened<append_req_t>(&cpy, &tags[0]);
  if (ret != SGX_SUCCESS)
    return ret;

  ret = get_mac_list<entry_t>(append_req->entries, append_req->entries_n, &tags[1]);
  if (ret != SGX_SUCCESS)
    return ret;

  return get_mac_list<sgx_cmac_128bit_tag_t>(tags, 2, tag);
}

sgx_status_t set_mac_append_req(append_req_t* append_req) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = get_mac_append_req(append_req, &tag);
  memcpy(append_req->mac, tag, SGX_CMAC_MAC_SIZE);

  return ret;
}

sgx_status_t validate_append_req(append_req_t* append_req, bool& valid) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = get_mac_append_req(append_req, &tag);
  valid = cmp_macs(append_req->mac, tag);

  return ret;
}