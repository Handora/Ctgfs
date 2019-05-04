add_library(sstable sstable.cc sst_meta.cc flusher.cc)
target_link_libraries(sstable log)
