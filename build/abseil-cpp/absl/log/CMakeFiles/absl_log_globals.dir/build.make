# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.22

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /global/home/groups/consultsw/sl-7.x86_64/modules/cmake/3.22.0/bin/cmake

# The command to remove a file.
RM = /global/home/groups/consultsw/sl-7.x86_64/modules/cmake/3.22.0/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /global/home/users/pmundra/graph-dp/src

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /global/home/users/pmundra/graph-dp/build

# Include any dependencies generated for this target.
include abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/depend.make
# Include any dependencies generated by the compiler for this target.
include abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/compiler_depend.make

# Include the progress variables for this target.
include abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/progress.make

# Include the compile flags for this target's objects.
include abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/flags.make

abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o: abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/flags.make
abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o: /global/home/users/pmundra/graph-dp/src/abseil-cpp/absl/log/globals.cc
abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o: abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/compiler_depend.ts
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/global/home/users/pmundra/graph-dp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o"
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && /global/software/sl-7.x86_64/modules/langs/gcc/12.1.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -MD -MT abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o -MF CMakeFiles/absl_log_globals.dir/globals.cc.o.d -o CMakeFiles/absl_log_globals.dir/globals.cc.o -c /global/home/users/pmundra/graph-dp/src/abseil-cpp/absl/log/globals.cc

abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/absl_log_globals.dir/globals.cc.i"
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && /global/software/sl-7.x86_64/modules/langs/gcc/12.1.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /global/home/users/pmundra/graph-dp/src/abseil-cpp/absl/log/globals.cc > CMakeFiles/absl_log_globals.dir/globals.cc.i

abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/absl_log_globals.dir/globals.cc.s"
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && /global/software/sl-7.x86_64/modules/langs/gcc/12.1.0/bin/g++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /global/home/users/pmundra/graph-dp/src/abseil-cpp/absl/log/globals.cc -o CMakeFiles/absl_log_globals.dir/globals.cc.s

# Object files for target absl_log_globals
absl_log_globals_OBJECTS = \
"CMakeFiles/absl_log_globals.dir/globals.cc.o"

# External object files for target absl_log_globals
absl_log_globals_EXTERNAL_OBJECTS =

abseil-cpp/absl/log/libabsl_log_globals.a: abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/globals.cc.o
abseil-cpp/absl/log/libabsl_log_globals.a: abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/build.make
abseil-cpp/absl/log/libabsl_log_globals.a: abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/global/home/users/pmundra/graph-dp/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libabsl_log_globals.a"
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && $(CMAKE_COMMAND) -P CMakeFiles/absl_log_globals.dir/cmake_clean_target.cmake
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/absl_log_globals.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/build: abseil-cpp/absl/log/libabsl_log_globals.a
.PHONY : abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/build

abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/clean:
	cd /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log && $(CMAKE_COMMAND) -P CMakeFiles/absl_log_globals.dir/cmake_clean.cmake
.PHONY : abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/clean

abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/depend:
	cd /global/home/users/pmundra/graph-dp/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /global/home/users/pmundra/graph-dp/src /global/home/users/pmundra/graph-dp/src/abseil-cpp/absl/log /global/home/users/pmundra/graph-dp/build /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log /global/home/users/pmundra/graph-dp/build/abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : abseil-cpp/absl/log/CMakeFiles/absl_log_globals.dir/depend

