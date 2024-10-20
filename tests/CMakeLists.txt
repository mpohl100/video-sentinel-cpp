find_package(Catch2 REQUIRED)

find_package( OpenCV REQUIRED )

add_executable(tests tests.cpp webcam.cpp object.cpp slices.cpp preview.cpp trace.cpp)
target_link_libraries(tests webcam Catch2::Catch2WithMain ${OpenCV_LIBS})
target_include_directories(tests PUBLIC ${OpenCV_INCLUDE_DIRS})
target_include_directories(tests PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_compile_definitions(tests PRIVATE CMAKE_SRC_DIR="${CMAKE_SOURCE_DIR}")
add_test(NAME test COMMAND $<TARGET_FILE:tests>)
