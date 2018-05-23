#pragma once
#include <string>
#include "msg.h"

// UNSAFE! Only for PoC. Should be provisioned by remote attestation with IAS.
const sgx_cmac_128bit_key_t p_key = { 0x6b, 0x90, 0x12, 0x73, 0x0F, 0x3e, 0x18, 0xe5, 0x34, 0xa6, 0x26, 0x5, 0x3b, 0x6c, 0xA2, 0x1d };

bool cmp_macs(sgx_cmac_128bit_tag_t mac_1, sgx_cmac_128bit_tag_t mac_2); 

template<typename T>
sgx_status_t get_mac_list(T* list, uint32_t length, sgx_cmac_128bit_tag_t* tag) {
  sgx_status_t ret = sgx_rijndael128_cmac_msg(&p_key, (uint8_t*)list, sizeof(T)*length, tag);
  return ret;
}

template<typename T>
sgx_status_t get_mac_flattened(T* msg, sgx_cmac_128bit_tag_t* tag) {
  sgx_status_t ret = sgx_rijndael128_cmac_msg(&p_key, (uint8_t*)msg, sizeof(T) - SGX_CMAC_MAC_SIZE, tag);
  return ret;
}

template<typename T>
sgx_status_t set_mac_flat_msg(T* msg) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = get_mac_flattened<T>(msg, &tag);
  memcpy(msg->mac, tag, SGX_CMAC_MAC_SIZE);
  return ret;
}

template<typename T>
sgx_status_t validate_flat_msg(T* msg, bool& valid) {
  sgx_cmac_128bit_tag_t tag;
  sgx_status_t ret = get_mac_flattened<T>(msg, &tag);
  valid = cmp_macs(msg->mac, tag);
  return ret;
}

sgx_status_t set_mac_append_req(append_req_t* append_req);

sgx_status_t validate_append_req(append_req_t* append_req, bool& valid);