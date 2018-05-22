#pragma once
#include <stdint.h>
#include <vector>
#include <map>

using namespace std;

struct UID {
	uint64_t id1; //most significant
	uint64_t id2; //least significant
};

struct cmp_uids {
	bool operator()(const UID& lhs, const UID& rhs) const {
		if (lhs.id1 == rhs.id1)
			return lhs.id2 < rhs.id2;

		return lhs.id1 < rhs.id1;
	}
};

struct relation_set {
	vector<UID> excludes;
	vector<UID> includes;
	vector<UID> responses;
	vector<UID> conditions;
	vector<UID> milestones;
};

struct dcr_event {
	UID id;
	string name;

	bool executed;
	bool excluded;
	bool pending;

	//relations that effect state or enabledness of this event
	relation_set incoming_relations;

	//relations that this event effect state or enabledness of
	relation_set outgoing_relations;
};

struct dcr_workflow {
	UID id;
	string name;
	map<UID, dcr_event, cmp_uids> event_store;
};

bool is_event_enabled(UID event_id);

void set_event_executed(UID event_id);
