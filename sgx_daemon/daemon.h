#pragma once
#include <grpcpp/grpcpp.h>
#include "protos/sgxd.grpc.pb.h"
//#include "protos/netd.grpc.pb.h"

class daemon {
public:
	std::shared_ptr<grpc::ChannelInterface> net_channel;
	//std::unique_ptr<tdcr::netd::NetDaemon::Stub> net_stub;
	unsigned short port;

	daemon(unsigned short port, unsigned short net_port);

	void start_daemon();
	void net_async_send(tdcr::network::Container& cont);
	void net_register();

	class SgxDaemonImpl final : public tdcr::sgxd::SgxDaemon::Service {
		// Confiure the SgxDaemon and start operating
		grpc::Status Config(grpc::ServerContext* context, const tdcr::sgxd::SgxConfig* conf, google::protobuf::Empty* response) override;

		// Send a message to the underlying enclave
		// If target is not configured self, the message is dropped
		grpc::Status Send(grpc::ServerContext* context, const tdcr::network::Container* cont, google::protobuf::Empty* response) override;
	};
};
