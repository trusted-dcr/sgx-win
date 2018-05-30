#pragma once
#include <string>
#include <sstream>
#include <iomanip>
#include "msg.h"

template <typename T> std::string to_hex(T t, size_t hex_len = sizeof(T) << 1) {
	static const char* digits = "0123456789ABCDEF";
	char* c = (char*)&t;
	std::string rc(hex_len, '0');
	for (size_t i = 0; i < hex_len; ++i)
		rc[i] = digits[i % 2 ? ((c[i / 2] & 0xF0) >> 4) : (c[i / 2] & 0x0F)];
	return "0x" + rc;
}

void print(std::string str) {
	print(str.c_str());
}

void printn(std::string str) {
	print(str + "\n");
}

std::string to_string(uid_t uid) {
	return to_hex(uid);
}

std::string to_string(command_type_t type) {
	switch (type) {
	case LOCK:
		return "LOCK";
	case ABORT:
		return "ABORT";
	case EXEC:
		return "EXEC";
	case CHECKPOINT:
		return "CHECKPOINT";
	default:
		throw;
	}
}

std::string to_string(entry_t entry) {
	return "(tag: " + to_string(entry.tag.uid)
		+ ", event: " + to_string(entry.event)
		+ ", type: " + to_string(entry.tag.type)
		+ ")";
}