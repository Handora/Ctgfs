add_library(fs fs.cc extent_server.cc extent_smain.cc)
add_library(fs_service fs_service.cc)
add_library(heart_beat_sender heart_beat_sender.cc)
target_link_libraries(fs_service proto)
target_link_libraries(fs_service brpc rpc)

add_executable(extent_server extent_server.cc extent_smain.cc)
add_library(info_detector info_detector.cc)
target_link_libraries(extent_server rpc brpc proto)
