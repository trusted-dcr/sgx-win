#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <sgx_tcrypto.h>

typedef struct uid_t {
  uint64_t id1;
  uint64_t id2;
} uid_t;

/* COMMAND TYPES */
typedef enum command_type_t {
  LOCK,
  ABORT,
  EXEC
} command_type_t;

typedef struct command_tag_t {
  uid_t uid;
  command_type_t type;
} command_tag_t;

typedef struct command_req_t {
  uid_t target;
  uid_t source;
  command_tag_t tag;
  uid_t event; 
  sgx_cmac_128bit_tag_t mac;
} command_req_t;

typedef struct command_rsp_t {
  uid_t target;
  uid_t source;
  command_tag_t tag;
  bool success;
  uid_t leader;
  sgx_cmac_128bit_tag_t mac;
} command_rsp_t;

/* APPEND TYPES */
typedef struct entry_t {
  uint32_t index;
  uint32_t term;
  uid_t event;
  uid_t source;
  command_tag_t tag;
} entry_t;

typedef struct append_req_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  uint32_t prev_term;
  uint32_t prev_index;
  uint32_t commit_index;
  entry_t* entries;
  uint32_t entries_n;
  sgx_cmac_128bit_tag_t mac;
} append_req_t;

typedef struct append_rsp_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  uint32_t prev_term;
  uint32_t prev_index;
  bool success;
  uint32_t last_index;
  sgx_cmac_128bit_tag_t mac;
} append_rsp_t;

/* POLL TYPES */
typedef struct poll_req_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  uint32_t last_term;
  uint32_t last_index;
  sgx_cmac_128bit_tag_t mac;
} poll_req_t;

typedef struct poll_rsp_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  bool success;
  sgx_cmac_128bit_tag_t mac;
} poll_rsp_t;

/* ELECTION TYPES */
typedef struct election_req_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  uint32_t last_term;
  uint32_t last_index;
  sgx_cmac_128bit_tag_t mac;
} election_req_t;

typedef struct election_rsp_t {
  uid_t target;
  uid_t source;
  uint32_t term;
  bool success;
  sgx_cmac_128bit_tag_t mac;
} election_rsp_t;
