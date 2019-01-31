add_library(fs fs.cc)
add_library(fs_service fs_service.cc)
add_library(heart_beat_sender heart_beat_sender.cc)
target_link_libraries(fs_service proto)
target_link_libraries(fs_service brpc)
target_link_libraries(heart_beat_sender proto)
target_link_libraries(heart_beat_sender brpc)
target_link_libraries(fs uuid)

add_definitions(-Ddev)

