add_library(master master.cc)
add_executable(lock_server handle.cc lock_server.cc lock_server_cache.cc lock_smain.cc)
target_link_libraries(master proto)
target_link_libraries(master brpc)
target_link_libraries(master rpc)
target_link_libraries(lock_server rpc)

add_library(gtg gtg.cc)
target_link_libraries(gtg rpc)
