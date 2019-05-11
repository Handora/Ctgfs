add_library(client client.cc extent_client.cc extent_client_cache.cc fuse.cc lock_client.cc lock_client_cache.cc yfs_client.cc)
target_link_libraries(client proto fuse protobuf)
target_link_libraries(client rpc)

add_library(client_cache extent_client.cc lock_client.cc extent_client_cache.cc lock_client_cache.cc)

add_executable(yfs_client extent_client.cc extent_client_cache.cc fuse.cc lock_client.cc lock_client_cache.cc yfs_client.cc client.cc)
target_link_libraries(yfs_client rpc fuse proto brpc)
add_library(gtc gtg_client.cc)
target_link_libraries(gtc rpc)
