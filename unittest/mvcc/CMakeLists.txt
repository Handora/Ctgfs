file(GLOB TEST_MVCC_SRCS "test_*.cc")
foreach(MVCC_T ${TEST_MVCC_SRCS})
    get_filename_component(MVCC_T_WE ${MVCC_T} NAME_WE)
    add_executable(${MVCC_T_WE} ${MVCC_T})
    target_link_libraries("${MVCC_T_WE}" mvcc mock_kv gtest_main)
    add_test(NAME "${MVCC_T_WE}" COMMAND "${MVCC_T_WE}")
endforeach()
