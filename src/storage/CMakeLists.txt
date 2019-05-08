add_subdirectory(sstable)
add_subdirectory(memtable)

add_library(log log.cc)
add_library(storage multi_iter.cc)
target_link_libraries(storage log)
