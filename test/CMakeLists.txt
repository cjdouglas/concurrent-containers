include(FetchGTest)

enable_testing()

set(
  SOURCES
  test_array.cc
  test_array_concurrent.cc
  test_vector.cc
)

add_executable(cds-tests ${SOURCES})
target_link_libraries(cds-tests PRIVATE cds-core GTest::gtest_main)
add_test(NAME cds-tests COMMAND cds-tests)
