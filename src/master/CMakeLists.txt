add_library(master master.cc)
add_library(heart_beat heart_beat.cc)
target_link_libraries(master proto)
target_link_libraries(heart_beat proto)
