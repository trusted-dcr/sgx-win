#pragma once
#include "msg.h"

// UNSAFE! Only for PoC. Should be provisioned by remote attestation with IAS.
sgx_cmac_128bit_key_t p_key = { 0x6b, 0x90, 0x12, 0x73, 0x0F, 0x3e, 0x18, 0xe5, 0x34, 0xa6, 0x26, 0x5, 0x3b, 0x6c, 0xA2, 0x1d };

template<typename T>
sgx_status_t set_mac_flat_msg(T* msg);

template<typename T>
sgx_status_t validate_flat_msg(T* msg, bool& valid);

//append_req_t is nested
sgx_status_t set_mac_append_req(append_req_t* append_req);

sgx_status_t validate_append_req(append_req_t* append_req, bool& valid);