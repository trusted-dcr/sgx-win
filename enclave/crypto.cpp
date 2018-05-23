#include <string.h>
#include "crypto.h"

bool cmp_macs(sgx_cmac_128bit_tag_t mac_1, sgx_cmac_128bit_tag_t mac_2) {
  for (int i = 0; i < SGX_CMAC_MAC_SIZE; i++) {
    if (mac_1[i] != mac_2[i])
      return false;
  }

  return true;
}

template<typename T>
sgx_status_t mac_list(T* list, uint32_t length, sgx_cmac_128bit_tag_t* tag) {
  sgx_status_t ret = sgx_rijndael128_cmac_msg(&p_key, (uint8_t*)list, sizeof(T)*length, &tag);
  return ret;
}

template<typename T>
sgx_status_t mac_flattened(T* msg, sgx_cmac_128bit_tag_t* tag) {
  sgx_status_t ret = sgx_rijndael128_cmac_msg(&p_key, (uint8_t*)msg, sizeof(T) - SGX_CMAC_MAC_SIZE, &tag);
  return ret;
}

template<typename T>
sgx_status_t mac_flat_msg(T* msg) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = mac_flattened<command_req_t>(msg, &tag);
  memcpy(msg->mac, tag, SGX_CMAC_MAC_SIZE);
  return ret;
}

template<typename T>
sgx_status_t validate_flat_msg(T* msg, bool& valid) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = mac_flattened<command_req_t>(msg, &tag);
  valid = cmp_macs(msg->mac, tag);
  return ret;
}

//append_req_t is nested
sgx_status_t mac_append_req(append_req_t* append_req, sgx_cmac_128bit_tag_t* tag) {
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
  sgx_status_t ret = mac_flattened<append_req_t>(&cpy, &tags[0]);
  if (ret != SGX_SUCCESS)
    return ret;

  sgx_cmac_128bit_tag_t tag2;
  ret = mac_list<entry_t>(append_req->entries, append_req->entries_n, &tags[1]);
  if (ret != SGX_SUCCESS)
    return ret;

  ret = mac_list<sgx_cmac_128bit_tag_t>(tags, 2, tag);
}

sgx_status_t set_mac_append_req(append_req_t* append_req) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = mac_append_req(append_req, &tag);
  memcpy(append_req->mac, tag, SGX_CMAC_MAC_SIZE);

  return ret;
}

sgx_status_t validate_append_req(append_req_t* append_req, bool& valid) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = mac_append_req(append_req, &tag);
  valid = cmp_macs(append_req->mac, tag);

  return ret;
}
