#pragma once
#include <stdint.h>
#include <stdbool.h>

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
	command_tag_t tag;
	uid_t event;
} command_req_t;

typedef struct command_rsp_t {
	command_tag_t tag;
	bool success;
	uid_t leader;
} command_rsp_t;

/* APPEND TYPES */
/* POLL TYPES */
/* ELECTION TYPES */
