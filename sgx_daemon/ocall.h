#pragma once
#include "msg.h"

void request_time();
void send_command_req(command_req_t req);
void send_command_rsp(command_rsp_t rsp);
void send_append_rsp(append_rsp_t rsp);
void send_poll_req(poll_req_t req);
void send_poll_rsp(poll_rsp_t rsp);
void send_election_req(election_req_t req);
void send_election_rsp(election_rsp_t rsp);
