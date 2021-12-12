cmake_minimum_required(VERSION 3.0.0)
project(sci_practice_21-22 VERSION 0.1.0)

include(CTest)
enable_testing()

find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} ) 
 
add_executable(main src/main.cpp src/shake_compensation.cpp src/Canny_Hough.cpp src/Gauss_threshold.cpp)
 
target_link_libraries( main ${OpenCV_LIBS} ) 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
