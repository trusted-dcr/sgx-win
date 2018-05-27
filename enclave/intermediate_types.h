#pragma once
#include "msg.h"

struct intermediate_dcr_worflow {
  uid_t* event_ids;
  uint32_t events_count;
  uid_t* excluded;
  uint32_t excluded_count;
  uid_t* pending;
  uint32_t pending_count;
  uid_t* executed;
  uint32_t executed_count;
  uid_t* dcr_conditions_out;
  uid_t* dcr_conditions_in;
  uint32_t conditions_count;
  uid_t* dcr_milestones_out;
  uid_t* dcr_milestones_in;
  uint32_t milestones_count;
  uid_t* dcr_includes_out;
  uid_t* dcr_includes_in;
  uint32_t includes_count;
  uid_t* dcr_excludes_out;
  uid_t* dcr_excludes_in;
  uint32_t excludes_count;
  uid_t* dcr_responses_out;
  uid_t* dcr_responses_in;
  uint32_t responses_count;
};