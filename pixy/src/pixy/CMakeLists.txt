cmake_minimum_required (VERSION 2.8)
project (pixy CXX)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake" )

# Add sources here... #
add_executable (pixy main.cpp)

# libpixyusb should always come before libboost and libusb #
target_link_libraries (pixy pixyusb)

find_package ( libpixyusb REQUIRED )
find_package ( libusb-1.0 REQUIRED )
find_package ( Boost 1.49 COMPONENTS thread system chrono REQUIRED)
find_package( OpenCV REQUIRED )

target_link_libraries (pixy ${Boost_LIBRARIES})
target_link_libraries (pixy ${LIBUSB_1_LIBRARY})
target_link_libraries (pixy ${OpenCV_LIBS} )

file(STRINGS "cmake/VERSION" LIBPIXY_VERSION)
add_definitions(-D__LIBPIXY_VERSION__="${LIBPIXY_VERSION}")

include_directories (src
                     include
                     ../../common
                     ${Boost_INCLUDE_DIR}
                     ${LIBUSB_1_INCLUDE_DIRS})

