add_library(parser parser.cc)
add_dependencies(parser third)
target_link_libraries(parser proto glog)
