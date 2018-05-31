# Trusted DCR
Distributed workflow execution with hardware-assisted Byzantine fault tolerance.

## Build prerequisites
* Visual Studio 2015
* Intel SGX SDK
* Google Protocol Buffers
* gGPC

Using vcpkg, the later two can be obtained as follows (for 64 bit):
```
vcpkg install protobuf:x64-windows
vcpkg install grpc:x64-windows
```

## Building
Make sure the `tdcr-proto` submodule is included by executing:
```
git submodule init
git submodule update
```

1. Run the `sgx_daemon\protos\build.bat` to build the protobuf types.
   * Using the build script requires environment variable `GRPC_PLUGIN_PATH` to point to a directory containing the gRPC C++ plugin `grpc_cpp_plugin.exe`. If using vcpkg, this will be something similar to `C:\vcpkg\installed\x64-windows\tools\grpc`.
   * Also requires `protoc` in PATH.
2. Build the `sgx_win` solution using Visual Studio 2015.
