file(GLOB TEST_PREFIX_TREE_SRCS "test_*.cc")

foreach(PREFIX_TREE_T ${TEST_PREFIX_TREE_SRCS})
  get_filename_component(PREFIX_TREE_T_WE ${PREFIX_TREE_T} NAME_WE)
  add_executable(${PREFIX_TREE_T_WE} ${PREFIX_TREE_T})
  target_link_libraries("${PREFIX_TREE_T_WE}" prefix_tree prefix_tree_node gtest_main)
  add_test(NAME "${PREFIX_TREE_T_WE}" COMMAND "${PREFIX_TREE_T_WE}") 
endforeach()
