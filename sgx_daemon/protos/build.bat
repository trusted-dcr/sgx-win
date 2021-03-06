@echo off

set PROTOC=protoc
set GRPC="%GRPC_PLUGIN_PATH%\grpc_cpp_plugin.exe"

set SOLUTION_DIR=..\..
set PROJECT_DIR=..

%PROTOC%^
  --proto_path=%SOLUTION_DIR%\tdcr-proto\protos^
  --cpp_out=%PROJECT_DIR%\protos^
  --grpc_out=%PROJECT_DIR%\protos^
  --plugin=protoc-gen-grpc=%GRPC%^
    %SOLUTION_DIR%\tdcr-proto\protos\crypto.proto^
    %SOLUTION_DIR%\tdcr-proto\protos\network.proto^
    %SOLUTION_DIR%\tdcr-proto\protos\raft.proto^
    %SOLUTION_DIR%\tdcr-proto\protos\dcr.proto^
    %SOLUTION_DIR%\tdcr-proto\protos\sgxd.proto
