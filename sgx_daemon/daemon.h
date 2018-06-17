#pragma once
#include <grpcpp/grpcpp.h>
#include "protos/sgxd.grpc.pb.h"
#include "msg_util.h"
#include "enclave_handle.h"
#include "convert.h"

class daemon {
public:
	class SgxDaemonImpl final : public tdcr::sgxd::SgxDaemon::Service {
	public:
		daemon* base;

		void init(daemon* base) {
			SgxDaemonImpl::base = base;
		}

		// Confiure the SgxDaemon and start operating
		grpc::Status Config(grpc::ServerContext* context, const tdcr::sgxd::SgxConfig* conf, google::protobuf::Empty* response) override;

		// Stop the SgxDaemon
		grpc::Status Stop(grpc::ServerContext* context, const google::protobuf::Empty* request, google::protobuf::Empty* response) override;

		// Send a message to the underlying enclave
		// If target is not configured self, the message is dropped
		grpc::Status Send(grpc::ServerContext* context, const tdcr::network::Container* cont, google::protobuf::Empty* response) override;

		// Execute a DCR event
		grpc::Status Execute(grpc::ServerContext* context, const tdcr::network::Uid* request, google::protobuf::Empty* response) override;

		// Run the cheapshot algorithm from this daemon
		grpc::Status History(grpc::ServerContext* context, const google::protobuf::Empty* request, tdcr::sgxd::Snapshot* response) override;
	};

	enclave_handle enclave;
	daemon::SgxDaemonImpl* rpc;
	std::map<uid_t, std::string, cmp_uids> addrs;
	unsigned short port;

	int wf_size;

	bool history_in_progress = false;
	bool history_ready = false;
	std::map<uid_t, std::vector<entry_t>, cmp_uids> history;

	daemon(uint16_t port);

	void start_daemon();

	template<typename T>
	void async_send(T msg) {
		async_send(convert::pack(msg));
	}

private:
	void async_send(tdcr::network::Container cont);
};
