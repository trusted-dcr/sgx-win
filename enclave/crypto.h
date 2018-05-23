#pragma once
#include "msg.h"

// UNSAFE! Only for PoC. Should be provisioned by remote attestation with IAS.
sgx_cmac_128bit_key_t p_key = { 0x6b, 0x90, 0x12, 0x73, 0x0F, 0x3e, 0x18, 0xe5, 0x34, 0xa6, 0x26, 0x5, 0x3b, 0x6c, 0xA2, 0x1d };

//command_req
sgx_status_t mac_command_req(command_req_t* command_req);
sgx_status_t validate_command_req(command_req_t* command_req, bool& valid);

//command_rsp
sgx_status_t mac_command_rsp(command_rsp_t* command_rsp);
sgx_status_t validate_command_rsp(command_rsp_t* command_rsp, bool& valid);

//append_req
sgx_status_t mac_append_req(append_req_t* append_req);
sgx_status_t validate_append_req(append_req_t* append_req, bool& valid);

//append_rsp
sgx_status_t mac_append_rsp(append_rsp_t* append_rsp);
sgx_status_t validate_append_rsp(append_rsp_t* append_rsp, bool& valid);

//poll_req
sgx_status_t mac_poll_req(poll_req_t* poll_req);
sgx_status_t validate_poll_req(poll_req_t* poll_req, bool& valid);

//poll_rsp
sgx_status_t mac_poll_rsp(poll_rsp_t* poll_rsp);
sgx_status_t validate_poll_rsp(poll_rsp_t* poll_rsp, bool& valid);

//election_req
sgx_status_t mac_election_req(election_req_t* election_req);
sgx_status_t validate_election_req(election_req_t* election_req, bool& valid);

//election_rsp
sgx_status_t mac_election_rsp(election_rsp_t* election_rsp);
sgx_status_t validate_election_rsp(election_rsp_t* election_rsp, bool& valid);
