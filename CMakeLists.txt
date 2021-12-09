cmake_minimum_required(VERSION 3.0.0)
project(sci_practice_21-22 VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
include_directories( shake_compensation.h )
include_directories( shake_compensation.cpp )

add_executable(horizon_detect1 src/Canny_Hough.cpp)
add_executable(horizon_detect2 src/Gauss_threshold.cpp)
add_executable(optical_flow src/sparse_optical_flow_test.cpp src/shake_compensation.cpp)

target_link_libraries( horizon_detect1 ${OpenCV_LIBS} )
target_link_libraries( horizon_detect2 ${OpenCV_LIBS} )
target_link_libraries( optical_flow ${OpenCV_LIBS} ) 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
