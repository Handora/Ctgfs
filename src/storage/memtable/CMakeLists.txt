add_library(memtable wal.cc memtable.cc)
target_link_libraries(memtable log sstable)
