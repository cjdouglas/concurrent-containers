include(FetchGTest)

enable_testing()

set(
  SOURCES
  test_cds_array.cc
)

add_executable(cds-tests ${SOURCES})
target_link_libraries(cds-tests PRIVATE cds-core GTest::gtest_main)
add_test(NAME cds-tests COMMAND cds-tests)
