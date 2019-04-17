add_library(master handle.cc lock_server.cc lock_server_cache.cc master.cc)
target_link_libraries(master proto)
target_link_libraries(master brpc)
target_link_libraries(master rpc)
