#pragma once
#include "dcr.h"

dcr_event simple_event(uid_t id);

dcr_workflow wf_simple_condition();
dcr_workflow wf_simple_milestone();
dcr_workflow wf_simple_include();
dcr_workflow wf_simple_exclude();
dcr_workflow wf_simple_response();