add_library(master master.cc)
add_executable(lock_server handle.cc lock_server.cc lock_server_cache.cc lock_smain.cc)
target_link_libraries(master proto)
target_link_libraries(master brpc)
target_link_libraries(master rpc)
target_link_libraries(master client_cache)
target_link_libraries(lock_server rpc)
add_executable(master_main master_main.cc)
target_link_libraries(master_main proto brpc rpc master info_collector)

add_library(gtg gtg.cc)
target_link_libraries(gtg rpc)

add_library(prefix_tree_node prefix_tree_node.cc)
add_library(prefix_tree prefix_tree.cc)
target_link_libraries(prefix_tree prefix_tree_node)

add_library(lock_server_for_test handle.cc lock_server.cc lock_server_cache.cc)