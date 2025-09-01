add_executable(mini_rpc main.cpp)

target_link_libraries(mini_rpc PUBLIC mini_rpc_library)

target_include_directories(mini_rpc PUBLIC ../../libraries/mini_rpc_library/inc)