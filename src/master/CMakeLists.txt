add_library(master master.cc)
target_link_libraries(master proto)
target_link_libraries(master brpc)
