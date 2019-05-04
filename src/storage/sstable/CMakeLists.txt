add_library(sst_meta sst_meta.cc)
target_link_libraries(sst_meta log)

add_library(sstable sstable.cc)
target_link_libraries(sstable sst_meta log)
