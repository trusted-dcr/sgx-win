#include <iomanip>
#include <winsock2.h>
#include "daemon.h"
#include "convert.h"
#include "dcr.cpp"

std::ostream& operator<< (std::ostream& stream, const uid_t& uid) {
	uint8_t* bytes = (uint8_t*)&uid;
	for (size_t i = 0; i < sizeof(uid_t); i++) {
		stream
			<< (i > 0 ? " " : "[")
			<< std::setfill('0')
			<< std::setw(2)
			<< std::hex
			<< (uint32_t)bytes[i];
	}
	stream << "]" << std::dec;
	return stream;
}

daemon::daemon(uint16_t port) {
	daemon::port = port;
	rpc = new daemon::SgxDaemonImpl;
}

void daemon::start_daemon() {
	// start enclave
	std::cout << "[INFO] Starting enclave" << std::endl;
	enclave.init_enclave();

	// start timer thread
	std::thread first([&]() {
		while (true) {
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			enclave.e_set_time();
		}
	});

	// start gRPC
	rpc->init(this);
	grpc::ServerBuilder builder;
	std::string addr = std::string("0.0.0.0:") + std::to_string(port);
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
	builder.RegisterService(rpc);
	std::cout << "[INFO] Starting sgxd on " << addr << std::endl;
	builder.BuildAndStart()->Wait();
}

void daemon::async_send(tdcr::network::Container cont) {
	std::thread([this, cont]() {
		uid_t target = convert::from_wire(cont.target());
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addrs[target], grpc::InsecureChannelCredentials());
		std::unique_ptr<tdcr::sgxd::SgxDaemon::Stub> stub = tdcr::sgxd::SgxDaemon::NewStub(channel);
		grpc::ClientContext context;
		google::protobuf::Empty response;
		grpc::Status status = stub->Send(&context, cont, &response);
		if (!status.ok())
			std::cout << "[ERR] General gRPC send error" << std::endl
								<< "  addr: " << addrs[target] << std::endl
								<< "  msg:  " << status.error_message() << std::endl
								<< "  code: " << status.error_code() << std::endl;
	}).detach();
}

/* SgxDaemonImpl */

grpc::Status daemon::SgxDaemonImpl::Config(grpc::ServerContext* context, const tdcr::sgxd::SgxConfig* conf, google::protobuf::Empty* response) {
	uid_t self = convert::from_wire(conf->self());
	dcr_workflow wf = convert::from_wire(conf->workflow());
	std::map<uid_t, uid_t, cmp_uids> peer_to_event;

	for each (tdcr::sgxd::SgxConfig::Peer peer in conf->peers()) {
		uid_t peer_uid = convert::from_wire(peer.uid());

		// event
		peer_to_event[peer_uid] = convert::from_wire(peer.event());

		// addr
		in_addr peer_ip;
		peer_ip.s_addr = peer.addr().ip();
		std::string peer_ip_str(inet_ntoa(peer_ip));
		base->addrs[peer_uid] = peer_ip_str + ":" + std::to_string(peer.addr().port());
	}

	std::cout << "[INFO] Received config from " << context->peer() << std::endl;
	std::cout << "  own uid: " << self << std::endl;
	std::cout << "  cluster: " << peer_to_event[self] << std::endl;
	std::cout << "  workflow size: " << conf->workflow().events_size() << std::endl;

	std::cout << "  routes:" << std::endl;
	for each (std::pair<uid_t, std::string> kv in base->addrs)
		std::cout << "    " << kv.first << " " << kv.second << std::endl;

	std::cout << "[INFO] Injecting config into enclave" << std::endl;
	base->enclave.e_configure_enclave(self, wf, peer_to_event);

	return grpc::Status::OK;
}

grpc::Status daemon::SgxDaemonImpl::Stop(grpc::ServerContext* context, const google::protobuf::Empty* request, google::protobuf::Empty* response) {
	exit(0);
}

grpc::Status daemon::SgxDaemonImpl::Send(grpc::ServerContext* context, const tdcr::network::Container* cont, google::protobuf::Empty* response) {
	switch (cont->type()) {
	case tdcr::network::Container::COMMAND_REQUEST: {
		std::cout << "[INFO] Received <COMMAND_REQUEST> from " << context->peer() << std::endl;
		command_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_command_req(req);
		break;
	}
	case tdcr::network::Container::COMMAND_RESPONSE: {
		std::cout << "[INFO] Received <COMMAND_RESPONSE> from " << context->peer() << std::endl;
		command_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_command_rsp(rsp);
		break;
	}
	case tdcr::network::Container::APPEND_REQUEST: {
		std::cout << "[INFO] Received <APPEND_REQUEST> from " << context->peer() << std::endl;
		append_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_append_req(req);
		break;
	}
	case tdcr::network::Container::APPEND_RESPONSE: {
		std::cout << "[INFO] Received <APPEND_RESPONSE> from " << context->peer() << std::endl;
		append_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_append_rsp(rsp);
		break;
	}
	case tdcr::network::Container::POLL_REQUEST: {
		std::cout << "[INFO] Received <POLL_REQUEST> from " << context->peer() << std::endl;
		poll_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_poll_req(req);
		break;
	}
	case tdcr::network::Container::POLL_RESPONSE: {
		std::cout << "[INFO] Received <POLL_RESPONSE> from " << context->peer() << std::endl;
		poll_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_poll_rsp(rsp);
		break;
	}
	case tdcr::network::Container::ELECTION_REQUEST: {
		std::cout << "[INFO] Received <ELECTION_REQUEST> from " << context->peer() << std::endl;
		election_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_election_req(req);
		break;
	}
	case tdcr::network::Container::ELECTION_RESPONSE: {
		std::cout << "[INFO] Received <ELECTION_RESPONSE> from " << context->peer() << std::endl;
		election_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_election_rsp(rsp);
		break;
	}
	case tdcr::network::Container::LOG_REQUEST: {
		std::cout << "[INFO] Received <LOG_REQUEST> from " << context->peer() << std::endl;
		log_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_log_req(req);
		break;
	}
	case tdcr::network::Container::LOG_RESPONSE: {
		std::cout << "[INFO] Received <LOG_RESPONSE> from " << context->peer() << std::endl;
		log_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_log_rsp(rsp);
		break;
	}
	default:
		std::cout << "[WARN] Unknown message received" << std::endl;
		break;
	}

	return grpc::Status::OK;
}

grpc::Status daemon::SgxDaemonImpl::Execute(grpc::ServerContext* context, const tdcr::network::Uid* event, google::protobuf::Empty* response) {
	base->enclave.e_execute(convert::from_wire(*event));
	return grpc::Status::OK;
}

grpc::Status daemon::SgxDaemonImpl::History(grpc::ServerContext* context, const google::protobuf::Empty* request, tdcr::sgxd::Snapshot* snapshot) {
	if (base->history_in_progress)
		return grpc::Status::CANCELLED;
	base->history_in_progress = true;

	// prompt enclave for history
	base->enclave.e_get_history();

	// spin to win
	auto timeout = std::chrono::system_clock::now() + std::chrono::seconds(30);
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));

		// return history to rpc client
		if (base->history_ready) {
			for each (auto cluster_pair in base->history) {
				auto part = snapshot->add_parts();
				part->set_allocated_cluster(new tdcr::network::Uid(convert::to_wire(cluster_pair.first)));
				for each (auto cluster_entry in cluster_pair.second) {
					auto entry = part->add_entries();
					entry->CopyFrom(convert::to_wire(cluster_entry));
				}
			}

			base->history_in_progress = false;
			base->history_ready = false;
			base->history.clear();
			return grpc::Status::OK;
		}

		// timeout
		if (std::chrono::system_clock::now() > timeout) {
			base->history_in_progress = false;
			return grpc::Status::CANCELLED;
		}
	}
}

daemon* daemon_instance;

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cout << "[ERR] Invalid port argument" << std::endl;
		return -1;
	}

	uint16_t port = atoi(argv[1]);
	daemon_instance = new daemon(port);
	daemon_instance->start_daemon();
	return 0;
}
