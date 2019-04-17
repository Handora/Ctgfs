add_library(client client.cc extent_client.cc extent_client_cache.cc fuse.cc lock_client.cc lock_client_cache.cc yfs_client.cc)
target_link_libraries(client proto fuse)
target_link_libraries(client parser rpc)
