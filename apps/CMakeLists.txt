include_directories(${CMAKE_SOURCE_DIR}/src)
link_directories(${CMAKE_SOURCE_DIR}/src)

find_package(clara REQUIRED)
find_package(OpenCV REQUIRED)

add_executable(webcam_app webcam_app.cpp)
target_include_directories(webcam_app PUBLIC ${OpenCV_INCLUDE_DIRS})
target_link_libraries(webcam_app clara::clara detection webcam ${OpenCV_LIBS})
target_include_directories(webcam_app PUBLIC ${CMAKE_SOURCE_DIR}/src)