cmake_minimum_required (VERSION 2.8)
project (camera CXX)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )
add_executable (camera main.cpp)
find_package( OpenCV REQUIRED )
target_link_libraries (camera ${OpenCV_LIBS} )


