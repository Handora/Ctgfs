file(GLOB TEST_COLLECTOR_SRCS "test_*.cc")
foreach(COLLECTOR_T ${TEST_COLLECTOR_SRCS})
    get_filename_component(COLLECTOR_T_WE ${COLLECTOR_T} NAME_WE)
    add_executable(${COLLECTOR_T_WE} ${COLLECTOR_T})
    target_link_libraries("${COLLECTOR_T_WE}" extent_server_for_test master rpc gtest_main)
    add_test(NAME "${COLLECTOR_T_WE}" COMMAND "${COLLECTOR_T_WE}")
endforeach()
