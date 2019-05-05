add_library(sstable sstable.cc sst_meta.cc flusher.cc sst_mgr.cc)
target_link_libraries(sstable log)
