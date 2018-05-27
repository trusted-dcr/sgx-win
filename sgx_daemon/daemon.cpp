#include <iomanip>
#include "daemon.h"
#include "convert.h"
#include <winsock2.h>

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
	uid_t target = convert::from_wire(cont.target());
	std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(addrs[target], grpc::InsecureChannelCredentials());
	std::unique_ptr<tdcr::sgxd::SgxDaemon::Stub> stub = tdcr::sgxd::SgxDaemon::NewStub(channel);

	grpc::ClientContext context;
	grpc::CompletionQueue queue;
	stub->AsyncSend(&context, cont, &queue);
}

/* SgxDaemonImpl */

grpc::Status daemon::SgxDaemonImpl::Config(grpc::ServerContext* context, const tdcr::sgxd::SgxConfig* conf, google::protobuf::Empty* response) {
	uid_t self = convert::from_wire(conf->self());
	uid_t cluster = { 0 };
	for each (tdcr::sgxd::SgxConfig::Peer peer in conf->peers()) {
		uid_t event = convert::from_wire(peer.event());
		if (event == self) {
			cluster = event;
			break;
		}
	}

	std::cout << "[INFO] Received config from " << context->peer() << std::endl;
	std::cout << "  own uid: " << self << std::endl;
	std::cout << "  cluster: " << cluster << std::endl;
	std::cout << "  workflow size: " << conf->workflow().events_size() << std::endl;

	for each (tdcr::sgxd::SgxConfig::Peer peer in conf->peers()) {
		uid_t peer_uid = convert::from_wire(peer.uid());
		in_addr peer_ip;
		peer_ip.s_addr = peer.addr().ip();
		std::string peer_ip_str(inet_ntoa(peer_ip));
		base->addrs[peer_uid] = peer_ip_str + ":" + std::to_string(peer.addr().port());
	}

	std::cout << "  routes:" << std::endl;
	for each (std::pair<uid_t, std::string> kv in base->addrs)
		std::cout << "    " << kv.first << " " << kv.second << std::endl;

	return grpc::Status::OK;
}

grpc::Status daemon::SgxDaemonImpl::Send(grpc::ServerContext* context, const tdcr::network::Container* cont, google::protobuf::Empty* response) {
	switch (cont->type()) {
	case tdcr::network::Container::COMMAND_REQUEST: {
		std::cout << "[INFO] <COMMAND_REQUEST> from " << context->peer() << std::endl;
		command_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_command_req(req);
		break;
	}
	case tdcr::network::Container::COMMAND_RESPONSE: {
		std::cout << "[INFO] <COMMAND_RESPONSE> from " << context->peer() << std::endl;
		command_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_command_rsp(rsp);
		break;
	}
	case tdcr::network::Container::APPEND_REQUEST: {
		std::cout << "[INFO] <APPEND_REQUEST> from " << context->peer() << std::endl;
		append_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_append_req(req);
		break;
	}
	case tdcr::network::Container::APPEND_RESPONSE: {
		std::cout << "[INFO] <APPEND_RESPONSE> from " << context->peer() << std::endl;
		append_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_append_rsp(rsp);
		break;
	}
	case tdcr::network::Container::POLL_REQUEST: {
		std::cout << "[INFO] <POLL_REQUEST> from " << context->peer() << std::endl;
		poll_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_poll_req(req);
		break;
	}
	case tdcr::network::Container::POLL_RESPONSE: {
		std::cout << "[INFO] <POLL_RESPONSE> from " << context->peer() << std::endl;
		poll_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_poll_rsp(rsp);
		break;
	}
	case tdcr::network::Container::ELECTION_REQUEST: {
		std::cout << "[INFO] <ELECTION_REQUEST> from " << context->peer() << std::endl;
		election_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_election_req(req);
		break;
	}
	case tdcr::network::Container::ELECTION_RESPONSE: {
		std::cout << "[INFO] <ELECTION_RESPONSE> from " << context->peer() << std::endl;
		election_rsp_t rsp;
		convert::unpack(*cont, rsp);
		base->enclave.e_recv_election_rsp(rsp);
		break;
	}
	case tdcr::network::Container::LOG_REQUEST: {
		std::cout << "[INFO] <LOG_REQUEST> from " << context->peer() << std::endl;
		log_req_t req;
		convert::unpack(*cont, req);
		base->enclave.e_recv_log_req(req);
		break;
	}
	case tdcr::network::Container::LOG_RESPONSE: {
		std::cout << "[INFO] <LOG_RESPONSE> from " << context->peer() << std::endl;
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
