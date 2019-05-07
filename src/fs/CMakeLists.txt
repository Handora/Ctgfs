add_library(fs_service fs_service.cc)
add_library(heart_beat_sender heart_beat_sender.cc)
target_link_libraries(fs_service proto)
target_link_libraries(fs_service brpc rpc)

add_executable(extent_server extent_server.cc extent_smain.cc)
add_library(info_collector info_collector.cc)
target_link_libraries(extent_server rpc brpc proto info_collector)

add_executable(heart_beat_sender_main heart_beat_sender_main.cc)
target_link_libraries(heart_beat_sender_main brpc proto heart_beat_sender master)

add_library(extent_server_for_test extent_server.cc)
target_link_libraries(extent_server_for_test rpc info_collector)