add_library(fs_service fs_service.cc)
target_link_libraries(fs_service proto)
target_link_libraries(fs_service brpc rpc)

add_executable(extent_server extent_server.cc extent_smain.cc)
target_link_libraries(extent_server rpc brpc proto info_collector)
add_library(info_collector info_collector.cc)
target_link_libraries(info_collector rpc)

add_library(extent_server_for_test extent_server.cc)
target_link_libraries(extent_server_for_test rpc info_collector)