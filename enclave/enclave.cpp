#include <string>
#include <sgx_trts.h>
#include "enclave_t.h"
#include "dcr.h"
#include "crypto.h"
#include "raft.h"

peer self;

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
    sgx_status_t status = (sgx_status_t)1; //placeholder
    while (status != SGX_SUCCESS) {
      status = set_mac_flat_msg<election_req_t>(&req);
    }
    election_messages.push_back(req);
  }

  // broadcast election messages
  for each (election_req_t req in election_messages) {
    send_election_req(req);
  }
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
      self.commit_index,      /* commit_index */
      placeholder,            /* entries */
      0,                      /* entries_n */
      { 0 }                   /* mac */
    };    
    sgx_status_t status = (sgx_status_t)1; //placeholder
    while (status != SGX_SUCCESS) {
      status = set_mac_append_req(&heartbeat);
    }
    heartbeats.push_back(heartbeat);
  }

  for each (append_req_t beat in heartbeats) {
    send_append_req(beat, beat.entries, 0);
  }
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
    sgx_status_t status = (sgx_status_t)1; //placeholder
    while (status != SGX_SUCCESS) {
      status = set_mac_flat_msg<poll_req_t>(&poll);
    }
    polls.push_back(poll);
  }

  for each (poll_req_t poll in polls) {
    send_poll_req(poll);
  }
}

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
      { 0 }           /* mac */
    };
    sgx_status_t status = set_mac_flat_msg<command_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;
    send_command_rsp(rsp);
    return;
  }

  //leader_recv logic:
  entry_t entry = {
    self.log.size(),  /* index */
    self.term,        /* term */
    req.event,        /* event */
    req.source,       /* source */
    req.tag           /* tag */
  };
  bool duplicate = self.exist_in_log(req.tag, entry);
  if (duplicate) { //duplicate command
    command_rsp_t rsp = {
      req.source,   /* target */
      self.id,      /* source */
      req.tag,      /* tag */
      true,         /* success */
      self.leader,  /* leader */
      {0}           /* mac */
    };
    sgx_status_t status = set_mac_flat_msg<command_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;
    send_command_rsp(rsp);
    return;
  }

  //new command
  self.log.push_back(entry);

  std::vector<append_req_t> appends;
  for each (uid_t member in self.cluster_members) {
    append_req_t req = {
      member,                   /* target */
      self.id,                  /* source */
      self.term,                /* term */
      self.last_entry().term,   /* prev_term */
      self.last_entry().index,  /* prev_index */
      self.commit_index,        /* commit_index */
      &entry,                   /* entries */
      1,                        /* entries_n */
      { 0 }                     /* mac */
    };
    sgx_status_t status = (sgx_status_t)1; //placeholder
    while (status != SGX_SUCCESS) {
      status = set_mac_append_req(&req);
    }
    appends.push_back(req);
  }

  for each (append_req_t msg in appends) {
    send_append_req(msg, msg.entries, msg.entries_n);
  }

  update_timeout(); // essentially a heartbeat, so skip one.
}

void recv_command_rsp(command_rsp_t rsp) { // not specified in peudocode. Needed for dcr/locking logic.

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
    sgx_status_t status = set_mac_flat_msg<append_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;

    send_append_rsp(rsp);
  }
  else { // we're up to date, update log with new entries
    self.remove_elements_after(req.prev_index);
    for (uint32_t i = 0; i < req.entries_n; i++) {
      self.log.push_back(req.entries[i]);
    }

    if (req.commit_index > self.commit_index) {
      self.commit_index = std::min(req.commit_index, (uint32_t)self.log.size());
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

    sgx_status_t status = set_mac_flat_msg<append_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;

    send_append_rsp(rsp);
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
    if (self.log[N].term == self.term && N > self.commit_index)
      self.commit_index = N;
  }
  else { // follower behind, send missing entries
    entry_t prev = self.log[rsp.prev_index - 1]; //check indexing!?
    entry_t* entries = &(self.log[rsp.prev_index]);
    uint32_t entries_n = self.log.size() - rsp.prev_index;
    append_req_t req = {
      rsp.source,   /* target */
      self.id,            /* source */
      self.term,          /* term */
      prev.term,          /* prev_term */
      prev.index,         /* prev_index */
      self.commit_index,  /* commit_index */ 
      entries,      /* entries */
      entries_n,    /* entries_n */
      { 0 }         /* mac */
    };
    sgx_status_t status = set_mac_append_req(&req);
    if (status != SGX_SUCCESS)
      return;
    send_append_req(req, entries, entries_n*sizeof(entry_t));
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
  sgx_status_t status = set_mac_flat_msg<poll_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    return;
  send_poll_rsp(rsp);
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
  sgx_status_t status = set_mac_flat_msg<election_rsp_t>(&rsp);
  if (status != SGX_SUCCESS)
    return;

  if (valid_election_request)
    self.endorsement = req.source;
    
  send_election_rsp(rsp);
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

  self.role = LEADER;
}

void timeout() {
  if (self.role == LEADER) {
    heartbeat();
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


void intialize() {
  //self.time_lock = new sgx_thread_mutex_t();
  //sgx_thread_mutex_init(self.time_lock,NULL);
  //self.time_lock->m_control = SGX_THREAD_MUTEX_RECURSIVE;
}
