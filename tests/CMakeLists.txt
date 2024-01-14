find_package(Catch2 REQUIRED)

add_executable(tests tests.cpp unittest.cpp)
target_link_libraries(tests lib Catch2::Catch2WithMain)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
add_test(NAME test COMMAND $<TARGET_FILE:tests>)