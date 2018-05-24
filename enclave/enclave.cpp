#include <string>
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

void recv_command_req(command_req_t req) {
  bool valid = verify_mac_and_own_term<command_req_t>(&req, validate_append_req);
  if (valid) {
    valid = verify_msg_term<command_req_t, command_rsp_t>(req, set_mac_flat_msg <append_rsp_t>, send_append_rsp);
  }
  if (!valid)
    return;

  //leader_recv logic:
  if (self.role == LEADER) {
    entry_t entry = {
      self.log.size(),/* index */
      self.term, /* term */
      req.event, /* event */
      req.source, /* source */
      req.tag/* tag */
    };
    bool duplicate = self.exist_in_log(req.tag, entry);
    if (duplicate) {
      command_rsp_t rsp = {
        req.source, /* uid_t target */
        self.id, /* uid_t source */
        req.tag, /* command_tag_t tag */
        true, /* bool success */
        self.leader, /* uid_t leader */
        {0} /* sgx_cmac_128bit_tag_t mac */
      };
      sgx_status_t status = set_mac_flat_msg<command_rsp_t>(&rsp);
      if (status != SGX_SUCCESS)
        return;
      send_command_rsp(rsp);
    }
    else { //new command
      self.log.push_back(entry);

      std::vector<append_req_t> appends;
      for each (uid_t member in self.cluster_members) {
        append_req_t req = {
          member,   /* target */
          self.id,            /* source */
          self.term,          /* term */
          self.last_entry().term,          /* prev_term */
          self.last_entry().index,         /* prev_index */
          self.commit_index,  /* commit_index */
          &entry,      /* entries */
          1,    /* entries_n */
          { 0 }         /* mac */
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

      //reset heartbeat
    }
  }

}

void recv_command_rsp(command_rsp_t rsp) {

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

  // follower_recv logic
  if (req.term == self.term && self.role == FOLLOWER) {
    if (uids_equal(req.source, self.leader))
      update_timeout();
    
    if (!self.exist_in_log(req.prev_term, req.prev_index)) {
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
    else {
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
}

void recv_append_rsp(append_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<append_rsp_t>(&rsp, validate_flat_msg<append_rsp_t>);
  
  if (!valid)
    return;

  //leader_recv logic
  if (self.role == LEADER) {
    if (rsp.success) {
      self.indices[rsp.source] = rsp.last_index;
      uint32_t N = self.largest_majority_index();
      if (self.log[N].term == self.term && N > self.commit_index)
        self.commit_index = N;
    }
    else { // send missing entries
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

}

void recv_poll_req(poll_req_t req) {
  bool valid = verify_mac_and_own_term<poll_req_t>(&req, validate_flat_msg<poll_req_t>);
  if (valid)
    valid = verify_msg_term<poll_req_t, poll_rsp_t>(req, set_mac_flat_msg<poll_rsp_t>,send_poll_rsp);
  if (!valid)
    return;

  // follower_recv logic
  if (self.role == FOLLOWER) {
    bool* timeout = new bool();
    sgx_status_t status = is_timed_out(timeout);

    poll_rsp_t rsp = {
      req.source,   /*target*/
      self.id,      /*source*/
      self.term,    /*term*/
      (uids_equal(self.endorsement, empty_uid) && // we have not voted yet AND
      (self.last_entry().term < req.last_term || // ( their term is higher OR
      self.last_entry().term == req.last_term && // their term is same AND
      self.last_entry().index <= req.last_index) && //their INDEX is higher) AND
      timeout),    /*success*/                 // timed out (min time)
      {0}           /*mac*/
    };
    status = set_mac_flat_msg<poll_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;
    send_poll_rsp(rsp);
  }
}

void recv_poll_rsp(poll_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<poll_rsp_t>(&rsp, validate_flat_msg<poll_rsp_t>);
  if (!valid)
    return;

  if (self.role == FOLLOWER) {
    if (rsp.success)
      self.poll_votes++;
    if (self.poll_votes > self.cluster_size / 2) {
      start_election();      
    }
  }
}

void recv_election_req(election_req_t req) {
  bool valid = verify_mac_and_own_term<election_req_t>(&req, validate_flat_msg<election_req_t>);
  if(valid)
    valid = verify_msg_term<election_req_t, election_rsp_t>(req, set_mac_flat_msg<election_rsp_t>, send_election_rsp);
  if (!valid)
    return;

  // follower.recv logic:
  if (self.role == FOLLOWER) {
    bool valid_election_request = uids_equal(self.endorsement, empty_uid) && //// we have not voted yet AND
                                  (self.last_entry().term < req.last_term || // ( their term is higher OR
                                  self.last_entry().term == req.last_term && // their term is same AND
                                  self.last_entry().index <= req.last_index); //their INDEX is higher)
    election_rsp_t rsp = {
      req.source ,/* target */
      self.id, /* source */
      self.term, /* term */
      valid_election_request,/* success */
      {0}/* mac */
    };
    sgx_status_t status = set_mac_flat_msg<election_rsp_t>(&rsp);
    if (status != SGX_SUCCESS)
      return;

    if (valid_election_request)
      self.endorsement = req.source;
    
    send_election_rsp(rsp);
  }

}

void recv_election_rsp(election_rsp_t rsp) {
  bool valid = verify_mac_and_own_term<election_rsp_t>(&rsp, validate_flat_msg<election_rsp_t>);
  if (!valid)
    return;

  //candidate_recv logic:
  if (self.role == CANDIDATE) {
    if (rsp.success) {
      self.election_votes++;

      if (self.election_votes > self.cluster_size / 2) {
        std::vector<append_req_t> victory_msgs;
        for each (uid_t member in self.cluster_members) {
          if (uids_equal(member, self.id))
            continue;
          entry_t* placeholder = new entry_t[1];
          placeholder[0] = { 0 };
          append_req_t req = {
            member, /* target */
            self.id, /* source */
            self.term, /* term */
            self.last_entry().term, /* prev_term */
            self.last_entry().index, /* prev_index */
            self.commit_index, /* commit_index */
            placeholder, /* entries */
            0, /* entries_n */
            {0}/* mac */
          };
          sgx_status_t status = (sgx_status_t)1; //placeholder
          while (status != SGX_SUCCESS) {
            status = set_mac_append_req(&req);
          }
          victory_msgs.push_back(req);
        }

        for each (append_req_t msg in victory_msgs) {
          send_append_req(msg, msg.entries, 0);
        }

        self.role = LEADER;
      }
    }
  }

}

void timeout() {

}

void intialize() {
  
}
