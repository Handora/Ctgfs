find_library(BRAFT_LIB NAMES libbraft.a)

add_library(fsm fsm.cc)
target_link_libraries(fsm ${BRAFT_LIB} glog gflags rocksdb)

