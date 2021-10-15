cmake_minimum_required(VERSION 3.0.0)
project(opencv_practice VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )

add_executable(opencv_practice Canny_Hough.cpp)
add_executable(opencv_practice2 Gauss_threshold.cpp)

target_link_libraries( opencv_practice ${OpenCV_LIBS} )
target_link_libraries( opencv_practice2 ${OpenCV_LIBS} )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
