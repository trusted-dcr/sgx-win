#include "daemon.h"

daemon::daemon(unsigned short port, unsigned short net_port) {
	daemon::port = port;

	net_channel = grpc::CreateChannel(
		std::string("localhost:") + std::to_string(net_port),
		grpc::InsecureChannelCredentials());

	net_stub = tdcr::netd::NetDaemon::NewStub(net_channel);
}

void daemon::start_daemon() {
	std::string addr = std::string("0.0.0.0:") + std::to_string(port);
	SgxDaemonImpl sgx_daemon;

	grpc::ServerBuilder builder;
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
	builder.RegisterService(&sgx_daemon);

	std::cout << "[INFO] Starting sgxd on " << addr << std::endl;
	builder.BuildAndStart()->Wait();
}

void daemon::net_async_send(tdcr::network::Container& cont) {
	grpc::ClientContext context;
	grpc::CompletionQueue queue;
	net_stub->AsyncSend(&context, cont, &queue);
}

void daemon::net_register() {
	grpc::ClientContext context;
	google::protobuf::Empty request;
	google::protobuf::Empty response;
	net_stub->Register(&context, request, &response);
}

grpc::Status daemon::SgxDaemonImpl::Config(grpc::ServerContext* context, const tdcr::sgxd::SgxConfig* conf, google::protobuf::Empty* response) {
	// TODO: ECALL
	return grpc::Status::CANCELLED;
}

grpc::Status daemon::SgxDaemonImpl::Send(grpc::ServerContext* context, const tdcr::network::Container* cont, google::protobuf::Empty* response) {
	// TODO: ECALL
	return grpc::Status::CANCELLED;
}
