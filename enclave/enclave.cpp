#include <string>
#include "enclave_t.h"
#include "dcr.h"
#include "raft.h"

peer self;

//helper functions
template<typename T, typename U>
bool verify_mac_and_own_term(T* msg, U validate_mac) {
  //validate mac
  bool is_valid_message = false;
  validate_mac(msg, is_valid_message);
  if (!is_valid_message)
    return false; //invalid mac

  //verify term
  if (msg->term > self.term) {
    self.update_term(msg->term, msg->source);
    self.locked_entry_index = -1;
    self.role = FOLLOWER;
    return false;
  }

  return true;
}

template<typename T, typename U, typename V, typename W>
bool verify_msg_term(T msg, V mac, W send) {
  if (msg.term < self.term) {
    U rsp = { 0 };
    rsp.source = self.id;
    rsp.target = msg.source;
    rsp.term = self.term;
    rsp.success = false;
    sgx_status_t status = mac(&rsp);
    if (status != SGX_SUCCESS)
      return false;

    send(rsp);
    return false;
  }
  return true;
}

//sending helpers
template<typename T, typename U, typename V>
void mac_and_broadcast_msgs(std::vector<T> msgs, U mac, V send) {
  for each (T msg in msgs) {
    sgx_status_t status;
    do {
      status = mac(&msg);
    } while (status != SGX_SUCCESS);
  }
  for each (T msg in msgs) {
    send(msg);
  }
}

void mac_and_broadcast_append_req(std::vector<append_req_t> msgs) {
  for each (append_req_t msg in msgs) {
    sgx_status_t status;
    do {
      status = set_mac_append_req(&msg);
    } while (status != SGX_SUCCESS);
  }
  for each (append_req_t msg in msgs) {
    send_append_req(msg,msg.entries,msg.entries_n);
  }
}

template<typename T, typename U, typename V>
void mac_and_send_msg(T msg, U mac, V send) {
  sgx_status_t status;
  do {
    status = mac(&msg);
  } while (status != SGX_SUCCESS);

  send(msg);
}

void mac_and_send_append_req(append_req_t msg) {
  sgx_status_t status;
  do {
    status = set_mac_append_req(&msg);
  } while (status != SGX_SUCCESS);

  send_append_req(msg,msg.entries,msg.entries_n);
}

void becomme_leader() {
  self.last_checkpoint = self.find_latest_checkpoint();
  uint32_t index;
  if (self.has_unresolved_lock(index)) {
    self.locked_entry_index = index;
  } 
  else {
    self.locked_entry_index = -1;
  }
  self.role = LEADER;
}

void update_timeout() {
  if (self.role == LEADER) {
    self.t = self.now + delta_heartbeat;
    return;
  }
  self.t_min = self.now + delta_min_time;

  unsigned char* rand = new unsigned char[2];
  uint16_t r;
  do {
    sgx_read_rand(rand, 2);
    r = (rand[1] << 8) + rand[0];
  } while (r > delta_max_time);

  self.t = self.t_min + r;
}

bool min_time_exceeded() {
  return self.now > self.t_min;
}

void start_election() {
  update_timeout();
  self.update_term(self.term + 1, self.id);
  self.role = CANDIDATE;

  // construct election messages
  std::vector<election_req_t> election_messages;
  for (int i = 0; i < self.cluster_members.size(); i++) {
    uid_t member = self.cluster_members[i];
    if (uids_equal(member, self.id))
      continue;
    election_req_t req = {
      member, /* target */
      self.id, /* source */
      self.term, /* term */
      self.last_entry().term, /* last_term */
      self.last_entry().index,/* last_index */
      { 0 }  /* mac */
    };
    election_messages.push_back(req);
  }
  mac_and_broadcast_msgs<election_req_t>(election_messages, set_mac_flat_msg<election_req_t>, send_election_req);
}

void append_to_log(entry_t entry) {
  self.log.push_back(entry);

  std::vector<append_req_t> appends;
  for each (uid_t member in self.cluster_members) {
    append_req_t req = {
      member,                   /* target */
      self.id,                  /* source */
      self.term,                /* term */
      self.last_entry().term,   /* prev_term */
      self.last_entry().index,  /* prev_index */
      self.get_commit_index(),  /* commit_index */
      &entry,                   /* entries */
      1,                        /* entries_n */
      { 0 }                     /* mac */
    };
    appends.push_back(req);
  }
  mac_and_broadcast_append_req(appends);

  update_timeout(); // essentially a heartbeat, so skip one.
}

void retry_requests() {
  for each (std::pair<uid_t, command_req_t> req in self.missing_resp) {
    mac_and_send_msg<command_req_t>(req.second, set_mac_flat_msg<command_req_t>, send_command_req);
  }
  if (self.locked_entry_index >= 0) { //resend lock, if unlock was missed
    entry_t lock = self.log[self.locked_entry_index];
    command_rsp_t rsp = {
      lock.source, /* target */
      self.id, /* source */
      lock.tag, /* tag */
      true, /* success */
      self.id, /* leader */
      { 0 }/* mac */
    };
    mac_and_send_msg<command_rsp_t>(rsp, set_mac_flat_msg<command_rsp_t>, send_command_rsp);
  }
  else {
    if (self.missing_resp.size() == 0 && self.last_checkpoint < self.get_commit_index()) { //not locked, no missing responses, and updated since last checkpoint
      entry_t checkpoint = {
        self.log.size(),  /* index */
        self.term,        /* term */
        self.cluster_event,        /* event */
        self.id,       /* source */
        {
          generate_random_uid(),
          CHECKPOINT 
        }       /* tag */
      };
      append_to_log(checkpoint);
      self.last_checkpoint = checkpoint.index;
    }
  }
}

void send_lock_requests(uid_t tag_id) {
  std::vector<command_req_t> lock_reqs;
  std::set<uid_t, cmp_uids> lock_set = self.workflow.get_lock_set(self.cluster_event);
  for each (uid_t event_id in lock_set) {
    command_req_t req;
    uid_t event_leader;
    try {
      event_leader = self.leader_map.at(event_id);
    }
    catch (const std::exception&) { // no leader for the intended cluster
      for each (std::pair<uid_t, uid_t> event_peer in self.peer_to_event_map) {
        if (uids_equal(event_peer.second, event_id)) {
          self.leader_map[event_id] = event_peer.first;
          event_leader = event_peer.first;
        }
      }
    }
    req = {
      event_leader,       /* target */
      self.id,            /* source */
      {
        tag_id,           /*tag uid*/
        LOCK              /*tag type*/
      },                  /* tag */
      self.cluster_event, /* event */
      { 0 }               /* mac */
    };
    lock_reqs.push_back(req);
  }
  mac_and_broadcast_msgs<command_req_t>(lock_reqs, set_mac_flat_msg<command_req_t>, send_command_req);
}

void send_execution(uid_t tag_id) {
  std::vector<command_req_t> exec_reqs;
  std::set<uid_t, cmp_uids> lock_set = self.workflow.get_lock_set(self.cluster_event);
  std::set<uid_t, cmp_uids> inform_set = self.workflow.get_inform_set(self.cluster_event);
  for each (uid_t event_id in inform_set) {
    command_req_t req;
    uid_t event_leader;
    try {
      event_leader = self.leader_map.at(event_id);

    }
    catch (const std::exception&) { // no leader for the intended cluster
      for each (std::pair<uid_t, uid_t> event_peer in self.peer_to_event_map) {
        if (uids_equal(event_peer.second, event_id)) {
          self.leader_map[event_id] = event_peer.first;
          event_leader = event_peer.first;
        }
      }
    }
    req = {
      event_leader,       /* target */
      self.id,            /* source */
      {
        tag_id,           /*tag uid*/
        EXEC              /*tag type*/
      },                  /* tag */
      self.cluster_event, /* event */
      { 0 }               /* mac */
    };
    exec_reqs.push_back(req);
    if (lock_set.find(event_id) == lock_set.end()) {
      self.missing_resp[req.target] = req;
    }
  }
  mac_and_broadcast_msgs<command_req_t>(exec_reqs, set_mac_flat_msg<command_req_t>, send_command_req);
}


void abort_execution(uid_t tag_id) {
  std::vector<command_req_t> abort_reqs;
  std::set<uid_t, cmp_uids> lock_set = self.workflow.get_lock_set(self.cluster_event);
  for each (uid_t event_id in lock_set) {
    command_req_t req;
    uid_t event_leader;
    try {
      event_leader = self.leader_map.at(event_id);
      
    }
    catch (const std::exception&) { // no leader for the intended cluster
      for each (std::pair<uid_t,uid_t> event_peer in self.peer_to_event_map) {
        if (uids_equal(event_peer.second, event_id)) {
          self.leader_map[event_id] = event_peer.first;
          event_leader = event_peer.first;
        }
      }
    }
    req = {
      event_leader,       /* target */
      self.id,            /* source */
      {
        tag_id,           /*tag uid*/
        ABORT             /*tag type*/
      },                  /* tag */
      self.cluster_event, /* event */
      { 0 }               /* mac */
    };
    abort_reqs.push_back(req);
  }
  mac_and_broadcast_msgs<command_req_t>(abort_reqs, set_mac_flat_msg<command_req_t>, send_command_req);
}

void update_commit_index(uint32_t new_index) {
  
  std::vector<entry_t> newly_committed;
  for (uint32_t i = self.get_commit_index() + 1; i < new_index; i++) {
    newly_committed.push_back(self.log[i]);
  }

  if (self.role != LEADER) {
    for each (entry_t entry in newly_committed) {
      if (entry.tag.type == EXEC)
        self.workflow.set_event_executed(entry.event);
    }
    self.set_commit_index(new_index);
    return;
  }

  //leader
  std::vector<command_rsp_t> responses;
  for each (entry_t entry in newly_committed) {
    if (entry.tag.type == CHECKPOINT) { //placeholder entry as checkpoint
      self.last_checkpoint = entry.index;
      continue;
    }
    if (entry.tag.type == EXEC) {
      self.workflow.set_event_executed(entry.event);
    }

    if (uids_equal(entry.event, self.cluster_event)) { // we own the event
      switch (entry.tag.type) {
      case LOCK:
        send_lock_requests(entry.tag.uid);
        continue;
      case EXEC:
        continue; //do nothing -- executions have already been sent
      case ABORT:
        std::set<uid_t, cmp_uids> new_set;
        self.locked_events = new_set;
        self.locked_entry_index = -1;
        abort_execution(entry.tag.uid);
        continue;
      }
    }

    //we do not own the event
    command_rsp_t rsp;
    switch (entry.tag.type) {
    case LOCK: //lock have been handled at command_req
      rsp = {
        entry.source, /* target */
        self.id, /* source */
        entry.tag, /* tag */
        true, /* success */
        self.id, /* leader */
        { 0 }/* mac */
      };
      break;
    case EXEC:
      rsp = {
        entry.source, /* target */
        self.id, /* source */
        entry.tag, /* tag */
        true, /* success */
        self.id, /* leader */
        { 0 }/* mac */
      };
      break;
    case ABORT:
      if (uids_equal(entry.event, self.log[self.locked_entry_index].event))
        self.locked_entry_index = -1;
      break;
    }
    responses.push_back(rsp);
  }
  mac_and_broadcast_msgs<command_rsp_t>(responses, set_mac_flat_msg<command_rsp_t>, send_command_rsp);

  self.set_commit_index(new_index);
}

void heartbeat() {
  std::vector<append_req_t> heartbeats;
  entry_t* placeholder = new entry_t[1];  // to avoid leaking trusted data
  placeholder[0] = { 0 };                 // to avoid leaking trusted data
  for each (uid_t member in self.cluster_members) {
    if (uids_equal(member, self.id))
      continue;
    append_req_t heartbeat = {
      member,                 /* target */
      self.id,                /* source */
      self.term,              /* term */
      self.last_entry().term, /* prev_term */
      self.last_entry().index,/* prev_index */
      self.get_commit_index(),      /* commit_index */
      placeholder,            /* entries */
      0,                      /* entries_n */
      { 0 }                   /* mac */
    };    
    heartbeats.push_back(heartbeat);
  }
  mac_and_broadcast_append_req(heartbeats);
}

void start_poll() {
  std::vector<poll_req_t> polls;
  for each (uid_t member in self.cluster_members) {
    if (uids_equal(member, self.id))
      continue;
    poll_req_t poll = {
      member,                 /* target */
      self.id,                /* source */
      self.term,              /* term */
      self.last_entry().term, /* last_term */
      self.last_entry().index,/* last_index */
      { 0 }
    };
    polls.push_back(poll);
  }
  mac_and_broadcast_msgs<poll_req_t>(polls, set_mac_flat_msg<poll_req_t>, send_poll_req);
}
 
//ECALLS

void recv_command_req(command_req_t req) {
  bool valid = false; 
  validate_flat_msg<command_req_t>(&req, valid);
  if (!valid)
    return;

  if (self.role != LEADER) { //only leaders handle to command requests. Other roles redirect to leader.
    command_rsp_t rsp = {
      req.source,     /* target */
      self.id,        /* source */
      req.tag,        /* tag */
      false,          /* success */
      self.leader,    /* leader */
      //req.event,      /* event */
      { 0 }           /* mac */
    };
    mac_and_send_msg<command_rsp_t>(rsp, set_mac_flat_msg<command_rsp_t>, send_command_rsp);
    return;
  }

  //leader_recv logic:
  uint32_t index_of_duplicate;
  if (self.exist_in_log(req.tag, index_of_duplicate)) { //duplicate command
    if (index_of_duplicate <= self.get_commit_index()) { // entry is already committed -- send success to client
      command_rsp_t rsp = {
        req.source,   /* target */
        self.id,      /* source */
        req.tag,      /* tag */
        true,         /* success */
        self.leader,  /* leader */
        {0}           /* mac */
      };
      mac_and_send_msg<command_rsp_t>(rsp, set_mac_flat_msg<command_rsp_t>, send_command_rsp);
    }
    return;
  }

  // It is not a duplicate command:
  // check for abort cases, i.e. we're already locked from other place, or we're starting execution, but not enabled
  if (req.tag.type == LOCK) {
  //check for lock
    if (self.has_unresolved_lock()) { // our event is locked -- two locks is not allowed
      command_rsp_t rsp = {
        req.source,   /* target */
        self.id,      /* source */
        { 
          req.tag.uid, /*tag uid*/
          ABORT        /*tag type*/ 
        },            /* tag */
        true,         /* success */ // check for same lock as already performed
        self.leader,  /* leader */
        //req.event,    /* event*/
        { 0 }         /* mac */
      };
      mac_and_send_msg<command_rsp_t>(rsp, set_mac_flat_msg<command_rsp_t>, send_command_rsp);
      return;
    }
  // we're not locked -- check lock-event and possibly enabledness
    if (uids_equal(req.event, self.cluster_event)) { // we are being locked on our cluster's event id, i.e we're starting an executing 
      if (!self.workflow.is_event_enabled(self.cluster_event)) {  // we cannot execute our event, and must reject the command
        command_rsp_t rsp = {
          req.source,   /* target */
          self.id,      /* source */
          {
            req.tag.uid, /*tag uid*/
            ABORT       /*tag type*/
          },            /* tag */
          true,         /* success */ // check for same lock as already performed
          self.leader,  /* leader */
          //req.event,    /* event */
          { 0 }         /* mac */
        };
        mac_and_send_msg<command_rsp_t>(rsp, set_mac_flat_msg<command_rsp_t>, send_command_rsp);

        return;
      }
    }
  }

  // all other cases is simply an append to the log
  // entry to be appended in log
  entry_t entry = {
    self.log.size(),  /* index */
    self.term,        /* term */
    req.event,        /* event */
    req.source,       /* source */
    req.tag           /* tag */
  };

  //add entry to log, and update cluster
  append_to_log(entry);
}

void recv_command_rsp(command_rsp_t rsp) { // not specified in peudocode. Needed for dcr/locking logic.
  bool valid = false;
  validate_flat_msg<command_rsp_t>(&rsp, valid);
  if (!valid)
    return;

  // if we're not leader, the response is not for us, and our new leader will eventually resend request.
  if (self.role != LEADER) {
    return;
  }

  //if !success, resend to that cluster leader
  if (!rsp.success) { //replace leader in missing_resp and leader_map
    uid_t new_leader = rsp.leader;
    if (rsp.tag.type == EXEC && self.missing_resp.find(rsp.source) != self.missing_resp.end()) { // EXEC should be in missing_resp - change it to the new leader
      command_req_t req = self.missing_resp[rsp.source]; 
      self.missing_resp.erase(rsp.source);
      self.missing_resp[new_leader] = req;
    }
    self.leader_map[self.peer_to_event_map[rsp.source]] = new_leader;
    return;
  }

  //we're leader, and our request was successful
  entry_t entry;
  switch (rsp.tag.type) {
  case ABORT: // can only happen if we're executing and a lock was rejected
    if (!self.executing_own_event()) // spurious extra safety check
      return;
    entry = {
      (uint32_t)self.log.size(),    /* index */
      self.term,          /* term */
      self.cluster_event, /* event */
      self.id,            /* source */
      {
        generate_random_uid(),
        ABORT
      }                   /* tag */
    };
    append_to_log(entry);
    break;
  case EXEC:
    if(self.missing_resp.find(rsp.source) != self.missing_resp.end())
      self.missing_resp.erase(rsp.source); 
    break;
  case LOCK:
    uint32_t lock_index;
    if (!self.exist_in_log(rsp.tag, lock_index))
      return; //should never happen
    if (self.lock_is_resolved(lock_index)) { // if the lock has been released, tell them how
      entry_t resolve_entry;
      if (self.lock_resolve_is_committed(lock_index, resolve_entry)) {
        command_req_t req = {
          rsp.source,         /* target */
          self.id,            /* source */
          resolve_entry.tag,  /* tag */
          resolve_entry.event,/* event */
          { 0 }               /* mac */
          };
        mac_and_send_msg<command_req_t>(req, set_mac_flat_msg<command_req_t>, send_command_req);
      }
      return; // lock is resolved, and we have send respose if committed
    }

    //lock is not resolved
    self.locked_events.insert(self.peer_to_event_map[rsp.source]);
    if (self.locks_aquired()) { // we have gained all locks
      uid_t tag_id = generate_random_uid();
      send_execution(tag_id);
      entry_t entry = {
        self.log.size(),  /* index */
        self.term,        /* term */
        self.cluster_event,/* event */
        self.id,       /* source */
        {
          tag_id,
          EXEC
        }           /* tag */
      };
      append_to_log(entry);
    }
    break;
  }

}

void recv_append_req(append_req_t req) {
	// copy entries to protected memory
	entry_t* prot_entries = new entry_t[req.entries_n];
	memcpy(prot_entries, req.entries, sizeof(entry_t) * req.entries_n);
	req.entries = prot_entries;

  bool valid = verify_mac_and_own_term<append_req_t>(&req, validate_append_req);
  if (valid) {
    valid = verify_msg_term<append_req_t, append_rsp_t>(req, set_mac_flat_msg <append_rsp_t>, send_append_rsp);
  }
  if (!valid)
    return;

  if (self.role != FOLLOWER) { // only followers responds to append requests.
    return;
  }

  // follower_recv logic
  if (uids_equal(req.source, self.leader))
    update_timeout();
    
  if (!self.term_and_index_exist_in_log(req.prev_term, req.prev_index)) { // we're not up to date, requst old appends
    append_rsp_t rsp = {
      req.source,       /*target*/
      self.id,          /*source*/
      self.term,        /*term*/
      req.prev_term,    /*prev_term*/
      req.prev_index,   /*prev_index*/
      false,            /*success*/
      -1,               /*last_index*/
      { 0 }             /*mac*/
    };
    mac_and_send_msg<append_rsp_t>(rsp, set_mac_flat_msg<append_rsp_t>, send_append_rsp);
  }
  else { // we're up to date, update log with new entries
    self.remove_elements_after(req.prev_index);
    for (uint32_t i = 0; i < req.entries_n; i++) {
      self.log.push_back(req.entries[i]);
    }

    if (req.commit_index > self.get_commit_index()) {
      update_commit_index(std::min(req.commit_index, (uint32_t)self.log.size()-1));
    }

    append_rsp_t rsp = {
      req.source,         /*target*/
      self.id,            /*source*/
      self.term,          /*term*/
      req.prev_term,      /*prev_term*/
      req.prev_index,     /*prev_index*/
      true,               /*success*/
      self.log.size() -1, /*last_index*/
      { 0 }               /*mac*/
    };
    mac_and_send_msg<append_rsp_t>(rsp, set_mac_flat_msg<append_rsp_t>, send_append_rsp);
  }
}

void recv_append_rsp(append_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<append_rsp_t>(&rsp, validate_flat_msg<append_rsp_t>);
  
  if (!valid)
    return;

  if (self.role != LEADER) { //only leaders handles append responses
    return;
  }

  //leader_recv logic
  if (rsp.success) { //follower correctly appended new entries
    self.indices[rsp.source] = rsp.last_index;
    uint32_t N = self.largest_majority_index();
    if (self.log[N].term == self.term && N > self.get_commit_index())
      update_commit_index(N);
  }
  else { // follower behind, send missing entries
    entry_t prev = self.log[rsp.prev_index - 1]; //check indexing!?
    entry_t* entries = &(self.log[rsp.prev_index]);
    uint32_t entries_n = self.log.size()-1 - rsp.prev_index;
    append_req_t req = {
      rsp.source,   /* target */
      self.id,            /* source */
      self.term,          /* term */
      prev.term,          /* prev_term */
      prev.index,         /* prev_index */
      self.get_commit_index(),  /* commit_index */ 
      entries,      /* entries */
      entries_n,    /* entries_n */
      { 0 }         /* mac */
    };    
    mac_and_send_append_req(req);
  }
}

void recv_poll_req(poll_req_t req) {
  bool valid = verify_mac_and_own_term<poll_req_t>(&req, validate_flat_msg<poll_req_t>);
  if (valid)
    valid = verify_msg_term<poll_req_t, poll_rsp_t>(req, set_mac_flat_msg<poll_rsp_t>,send_poll_rsp);
  if (!valid)
    return;

  if (self.role != FOLLOWER) { //only followers respons to poll requests
    return;
  }
  // follower_recv logic
  bool valid_time = min_time_exceeded();

  poll_rsp_t rsp = {
    req.source,   /*target*/
    self.id,      /*source*/
    self.term,    /*term*/
    (uids_equal(self.endorsement, empty_uid) && // we have not voted yet AND
    (self.last_entry().term < req.last_term || // ( their term is higher OR
    self.last_entry().term == req.last_term && // their term is same AND
    self.last_entry().index <= req.last_index) && //their INDEX is higher) AND
    valid_time),  /*success*/                 // timed out (min time)
    {0}           /*mac*/
  };
  mac_and_send_msg<poll_rsp_t>(rsp, set_mac_flat_msg<poll_rsp_t>, send_poll_rsp);
}

void recv_poll_rsp(poll_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<poll_rsp_t>(&rsp, validate_flat_msg<poll_rsp_t>);
  if (!valid)
    return;


  if (self.role != FOLLOWER) { //only followers handles poll responses
    return;
  }

  if (rsp.success)
    self.poll_votes++;
  if (self.poll_votes > self.cluster_size / 2) {
    start_election();      
  }
}

void recv_election_req(election_req_t req) {
  bool valid = verify_mac_and_own_term<election_req_t>(&req, validate_flat_msg<election_req_t>);
  if(valid)
    valid = verify_msg_term<election_req_t, election_rsp_t>(req, set_mac_flat_msg<election_rsp_t>, send_election_rsp);
  if (!valid)
    return;

  if (self.role != FOLLOWER) { //only followers respond t
    return;
  }
  // follower.recv logic:
  bool valid_election_request = uids_equal(self.endorsement, empty_uid) && //// we have not voted yet AND
                                (self.last_entry().term < req.last_term || // ( their term is higher OR
                                self.last_entry().term == req.last_term && // their term is same AND
                                self.last_entry().index <= req.last_index); //their INDEX is higher)
  election_rsp_t rsp = {
    req.source,             /* target */
    self.id,                /* source */
    self.term,              /* term */
    valid_election_request, /* success */
    {0}                     /* mac */
  };

  if (valid_election_request)
    self.endorsement = req.source;

  mac_and_send_msg<election_rsp_t>(rsp, set_mac_flat_msg<election_rsp_t>, send_election_rsp);

}

void recv_election_rsp(election_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<election_rsp_t>(&rsp, validate_flat_msg<election_rsp_t>);
  if (!valid)
    return;

  //candidate_recv logic:
  if (self.role != CANDIDATE) { // only candidates handles election responses
    return;
  }
  if (!rsp.success) { //no new votes
    return;
  }
  self.election_votes++;

  if (!(self.election_votes > self.cluster_size / 2)) { // no majority yet
    return;
  }
  heartbeat(); //victorious heartbeat message

  becomme_leader();
}

void timeout() {
  if (self.role == LEADER) {
    heartbeat();
    retry_requests();
    update_timeout();
  }
  if (self.role == CANDIDATE) {
    start_election(); //try again
  }
  if (self.role == FOLLOWER) {
    //start poll
    update_timeout();
    start_poll();
  }

}

void set_time(uint64_t ticks) {
  self.now = ticks;
  if (self.now > self.t)
    timeout();
}


void provision_enclave(
  uid_t self_id,
  uid_t self_cluster_event,
  uid_t wf_id,
  char* wf_name,
  uid_t* event_ids, uint32_t events_count,
  uid_t* excluded, uint32_t excluded_count,
  uid_t* pending, uint32_t pending_count,
  uid_t* executed, uint32_t executed_count,
  uid_t* dcr_conditions_out, uid_t* dcr_conditions_in, uint32_t conditions_count,
  uid_t* dcr_milestones_out, uid_t* dcr_milestones_in, uint32_t milestones_count,
  uid_t* dcr_includes_out, uid_t* dcr_includes_in, uint32_t includes_count,
  uid_t* dcr_excludes_out, uid_t* dcr_excludes_in, uint32_t excludes_count,
  uid_t* dcr_responses_out, uid_t* dcr_responses_in, uint32_t responses_count,
  uid_t* peer_map_peers, uid_t* peer_map_events, uint32_t peer_map_count
 ){
  //recreate intermediate
  intermediate_dcr_worflow wf = {
    event_ids, /* event_ids */
    events_count, /* events_count */
    excluded, /* excluded */
    excluded_count,/* excluded_count */
    pending, /* pending */
    pending_count, /* pending_count */
    executed, /* executed */
    executed_count, /* executed_count */
    dcr_conditions_out, /* dcr_conditions_out */
    dcr_conditions_in, /* dcr_conditions_in */
    conditions_count, /* conditions_count */
    dcr_milestones_out, /* dcr_milestones_out */
    dcr_milestones_in, /* dcr_milestones_in */
    milestones_count, /* milestones_count */
    dcr_includes_out, /* dcr_includes_out */
    dcr_includes_in, /* dcr_includes_in */
    includes_count, /* includes_count */
    dcr_excludes_out, /* dcr_excludes_out */
    dcr_excludes_in, /* dcr_excludes_in */
    excluded_count, /* excludes_count */
    dcr_responses_out,/* dcr_responses_out */
    dcr_responses_in, /* dcr_responses_in */
    responses_count/* responses_count */
  };

  //transform intermediate
  self.workflow = self.workflow.make_workflow(wf,wf_name);
  self.workflow.id = wf_id;
  self.id = self_id;
  //dcr_workflow* wf = (dcr_workflow*)self_workflow;
  self.cluster_event = self_cluster_event;
}

#ifdef SGX_DEBUG
command_req_t test_set_mac_command_req(command_req_t req) {
  sgx_status_t status;
  status = set_mac_flat_msg<command_req_t>(&req);
  if (status != SGX_SUCCESS)
    throw - 100;
  return req;
}
command_rsp_t test_set_mac_command_rsp(command_rsp_t rsp) {
  sgx_status_t status;
  status = set_mac_flat_msg<command_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    throw - 100;
  return rsp;
}

append_req_t test_set_mac_append_req(append_req_t req) {
  sgx_status_t status;
  status = set_mac_append_req(&req);
  if (status != SGX_SUCCESS)
    throw - 100;
  return req;
}

append_rsp_t test_set_mac_append_rsp(append_rsp_t rsp) {
  sgx_status_t status;
  status = set_mac_flat_msg<append_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    throw - 100;
  return rsp;
}

poll_req_t test_set_mac_poll_req(poll_req_t req) {
  sgx_status_t status;
  status = set_mac_flat_msg<poll_req_t>(&req);
  if (status != SGX_SUCCESS)
    throw - 100;
  return req;
}

poll_rsp_t test_set_mac_poll_rsp(poll_rsp_t rsp) {
  sgx_status_t status;
  status = set_mac_flat_msg<poll_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    throw - 100;
  return rsp;
}

election_req_t test_set_mac_election_req(election_req_t req) {
  sgx_status_t status;
  status = set_mac_flat_msg<election_req_t>(&req);
  if (status != SGX_SUCCESS)
    throw - 100;
  return req;
}

election_rsp_t test_set_mac_election_rsp(election_rsp_t rsp) {
  sgx_status_t status;
  status = set_mac_flat_msg<election_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    throw - 100;
  return rsp;
}
#endif // SGX_DEBUF
