# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build

# Include any dependencies generated for this target.
include CMakeFiles/optical_flow.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/optical_flow.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/optical_flow.dir/flags.make

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o: CMakeFiles/optical_flow.dir/flags.make
CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o: ../src/optical_flow.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o -c /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/optical_flow.cpp

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/optical_flow.dir/src/optical_flow.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/optical_flow.cpp > CMakeFiles/optical_flow.dir/src/optical_flow.cpp.i

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/optical_flow.dir/src/optical_flow.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/optical_flow.cpp -o CMakeFiles/optical_flow.dir/src/optical_flow.cpp.s

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.requires:

.PHONY : CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.requires

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.provides: CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.requires
	$(MAKE) -f CMakeFiles/optical_flow.dir/build.make CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.provides.build
.PHONY : CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.provides

CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.provides.build: CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o


CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o: CMakeFiles/optical_flow.dir/flags.make
CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o: ../src/shake_compensation.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o -c /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/shake_compensation.cpp

CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/shake_compensation.cpp > CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.i

CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/shake_compensation.cpp -o CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.s

CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.requires:

.PHONY : CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.requires

CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.provides: CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.requires
	$(MAKE) -f CMakeFiles/optical_flow.dir/build.make CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.provides.build
.PHONY : CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.provides

CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.provides.build: CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o


CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o: CMakeFiles/optical_flow.dir/flags.make
CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o: ../src/Canny_Hough.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o -c /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Canny_Hough.cpp

CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Canny_Hough.cpp > CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.i

CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Canny_Hough.cpp -o CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.s

CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.requires:

.PHONY : CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.requires

CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.provides: CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.requires
	$(MAKE) -f CMakeFiles/optical_flow.dir/build.make CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.provides.build
.PHONY : CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.provides

CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.provides.build: CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o


CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o: CMakeFiles/optical_flow.dir/flags.make
CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o: ../src/Gauss_threshold.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o"
	/usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o -c /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Gauss_threshold.cpp

CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.i"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Gauss_threshold.cpp > CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.i

CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.s"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/src/Gauss_threshold.cpp -o CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.s

CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.requires:

.PHONY : CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.requires

CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.provides: CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.requires
	$(MAKE) -f CMakeFiles/optical_flow.dir/build.make CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.provides.build
.PHONY : CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.provides

CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.provides.build: CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o


# Object files for target optical_flow
optical_flow_OBJECTS = \
"CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o" \
"CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o" \
"CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o" \
"CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o"

# External object files for target optical_flow
optical_flow_EXTERNAL_OBJECTS =

optical_flow: CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o
optical_flow: CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o
optical_flow: CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o
optical_flow: CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o
optical_flow: CMakeFiles/optical_flow.dir/build.make
optical_flow: /usr/local/lib/libopencv_gapi.so.4.5.4
optical_flow: /usr/local/lib/libopencv_stitching.so.4.5.4
optical_flow: /usr/local/lib/libopencv_freetype.so.4.5.4
optical_flow: /usr/local/lib/libopencv_intensity_transform.so.4.5.4
optical_flow: /usr/local/lib/libopencv_fuzzy.so.4.5.4
optical_flow: /usr/local/lib/libopencv_stereo.so.4.5.4
optical_flow: /usr/local/lib/libopencv_aruco.so.4.5.4
optical_flow: /usr/local/lib/libopencv_barcode.so.4.5.4
optical_flow: /usr/local/lib/libopencv_img_hash.so.4.5.4
optical_flow: /usr/local/lib/libopencv_ccalib.so.4.5.4
optical_flow: /usr/local/lib/libopencv_structured_light.so.4.5.4
optical_flow: /usr/local/lib/libopencv_face.so.4.5.4
optical_flow: /usr/local/lib/libopencv_surface_matching.so.4.5.4
optical_flow: /usr/local/lib/libopencv_bgsegm.so.4.5.4
optical_flow: /usr/local/lib/libopencv_reg.so.4.5.4
optical_flow: /usr/local/lib/libopencv_xfeatures2d.so.4.5.4
optical_flow: /usr/local/lib/libopencv_videostab.so.4.5.4
optical_flow: /usr/local/lib/libopencv_superres.so.4.5.4
optical_flow: /usr/local/lib/libopencv_phase_unwrapping.so.4.5.4
optical_flow: /usr/local/lib/libopencv_tracking.so.4.5.4
optical_flow: /usr/local/lib/libopencv_xphoto.so.4.5.4
optical_flow: /usr/local/lib/libopencv_shape.so.4.5.4
optical_flow: /usr/local/lib/libopencv_dnn_superres.so.4.5.4
optical_flow: /usr/local/lib/libopencv_saliency.so.4.5.4
optical_flow: /usr/local/lib/libopencv_line_descriptor.so.4.5.4
optical_flow: /usr/local/lib/libopencv_hfs.so.4.5.4
optical_flow: /usr/local/lib/libopencv_bioinspired.so.4.5.4
optical_flow: /usr/local/lib/libopencv_wechat_qrcode.so.4.5.4
optical_flow: /usr/local/lib/libopencv_xobjdetect.so.4.5.4
optical_flow: /usr/local/lib/libopencv_dnn_objdetect.so.4.5.4
optical_flow: /usr/local/lib/libopencv_mcc.so.4.5.4
optical_flow: /usr/local/lib/libopencv_rgbd.so.4.5.4
optical_flow: /usr/local/lib/libopencv_dpm.so.4.5.4
optical_flow: /usr/local/lib/libopencv_rapid.so.4.5.4
optical_flow: /usr/local/lib/libopencv_datasets.so.4.5.4
optical_flow: /usr/local/lib/libopencv_plot.so.4.5.4
optical_flow: /usr/local/lib/libopencv_text.so.4.5.4
optical_flow: /usr/local/lib/libopencv_optflow.so.4.5.4
optical_flow: /usr/local/lib/libopencv_ximgproc.so.4.5.4
optical_flow: /usr/local/lib/libopencv_video.so.4.5.4
optical_flow: /usr/local/lib/libopencv_photo.so.4.5.4
optical_flow: /usr/local/lib/libopencv_quality.so.4.5.4
optical_flow: /usr/local/lib/libopencv_ml.so.4.5.4
optical_flow: /usr/local/lib/libopencv_highgui.so.4.5.4
optical_flow: /usr/local/lib/libopencv_videoio.so.4.5.4
optical_flow: /usr/local/lib/libopencv_imgcodecs.so.4.5.4
optical_flow: /usr/local/lib/libopencv_objdetect.so.4.5.4
optical_flow: /usr/local/lib/libopencv_dnn.so.4.5.4
optical_flow: /usr/local/lib/libopencv_calib3d.so.4.5.4
optical_flow: /usr/local/lib/libopencv_features2d.so.4.5.4
optical_flow: /usr/local/lib/libopencv_flann.so.4.5.4
optical_flow: /usr/local/lib/libopencv_imgproc.so.4.5.4
optical_flow: /usr/local/lib/libopencv_core.so.4.5.4
optical_flow: CMakeFiles/optical_flow.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Linking CXX executable optical_flow"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/optical_flow.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/optical_flow.dir/build: optical_flow

.PHONY : CMakeFiles/optical_flow.dir/build

CMakeFiles/optical_flow.dir/requires: CMakeFiles/optical_flow.dir/src/optical_flow.cpp.o.requires
CMakeFiles/optical_flow.dir/requires: CMakeFiles/optical_flow.dir/src/shake_compensation.cpp.o.requires
CMakeFiles/optical_flow.dir/requires: CMakeFiles/optical_flow.dir/src/Canny_Hough.cpp.o.requires
CMakeFiles/optical_flow.dir/requires: CMakeFiles/optical_flow.dir/src/Gauss_threshold.cpp.o.requires

.PHONY : CMakeFiles/optical_flow.dir/requires

CMakeFiles/optical_flow.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/optical_flow.dir/cmake_clean.cmake
.PHONY : CMakeFiles/optical_flow.dir/clean

CMakeFiles/optical_flow.dir/depend:
	cd /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22 /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22 /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build /home/mexanobar/programming/C++/sci_practice_21-22/sci_practice_21-22/build/CMakeFiles/optical_flow.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/optical_flow.dir/depend

