find_package(Catch2 REQUIRED)

add_executable(tests tests.cpp webcam.cpp object.cpp slices.cpp preview.cpp)
target_link_libraries(tests webcam Catch2::Catch2WithMain)
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(tests PRIVATE CMAKE_SRC_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME test COMMAND $<TARGET_FILE:tests>)
