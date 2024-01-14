link_directories(${CMAKE_SOURCE_DIR}/src)

find_package(clara REQUIRED)

set( EXCEUTABLE_NAME main)

add_executable(${EXCEUTABLE_NAME} main.cpp)
target_link_libraries(${EXCEUTABLE_NAME} lib clara::clara)
target_include_directories(${EXCEUTABLE_NAME} PUBLIC ${CMAKE_SOURCE_DIR}/src)