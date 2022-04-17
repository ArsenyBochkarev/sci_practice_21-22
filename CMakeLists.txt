cmake_minimum_required(VERSION 3.1.0)

project(sci_practice_21-22  VERSION 1.0.0 LANGUAGES CXX)
 
 
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} ) 

add_executable(main src/main.cpp src/shake_compensation.cpp src/Canny_Hough.cpp src/Gauss_threshold.cpp )
 
target_link_libraries(main Qt5::Widgets)
 
target_link_libraries( main ${OpenCV_LIBS} ) 


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

SET(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTOUIC ON)
SET(CMAKE_AUTORCC ON)

