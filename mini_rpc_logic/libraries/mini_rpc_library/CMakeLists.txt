add_library (mini_rpc_library STATIC inc/transport.h src/transport.cpp inc/channel.h src/channel.cpp inc/crc8.h src/crc8.cpp
inc/registry.h src/registry.cpp inc/client_rpc.h src/client_rpc.cpp inc/server_rpc.h src/server_rpc.cpp)

target_include_directories(mini_rpc_library PUBLIC inc)