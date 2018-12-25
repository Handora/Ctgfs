add_library(client client.cc)
target_link_libraries(client proto)
target_link_libraries(client parser)
