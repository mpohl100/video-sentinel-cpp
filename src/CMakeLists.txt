set( LIB_SOURCES Lib.cpp)

find_package( range-v3 REQUIRED )

add_library( lib SHARED ${LIB_SOURCES} )
target_link_libraries( lib range-v3::range-v3)
target_include_directories(lib PUBLIC {CMAKE_SOURCE_DIR}/src)