file(GLOB TEST_WAL_SRCS "test_*.cc")
foreach(WAL_T ${TEST_WAL_SRCS})
    get_filename_component(WAL_T_WE ${WAL_T} NAME_WE)
    add_executable(${WAL_T_WE} ${WAL_T})
    target_link_libraries("${WAL_T_WE}" wal gtest_main)
    add_test(NAME "${WAL_T_WE}" COMMAND "${WAL_T_WE}")
endforeach()
