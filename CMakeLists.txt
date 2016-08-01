cmake_minimum_required(VERSION 3.2.2)
project(Mongoose)

set(MONGOOSE_VERSION_MAJOR   "1")
set(MONGOOSE_VERSION_MINOR   "0")
set(MONGOOSE_VERSION_PATCH   "4")
set(MONGOOSE_VERSION "${MONGOOSE_VERSION_MAJOR}.
                      ${MONGOOSE_VERSION_MINOR}.
                      ${MONGOOSE_VERSION_PATCH}")

if(NOT WIN32)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold  "${Esc}[1m")
  set(Red         "${Esc}[31m")
  set(Green       "${Esc}[32m")
  set(Yellow      "${Esc}[33m")
  set(Blue        "${Esc}[34m")
  set(Magenta     "${Esc}[35m")
  set(Cyan        "${Esc}[36m")
  set(White       "${Esc}[37m")
  set(BoldRed     "${Esc}[1;31m")
  set(BoldGreen   "${Esc}[1;32m")
  set(BoldYellow  "${Esc}[1;33m")
  set(BoldBlue    "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan    "${Esc}[1;36m")
  set(BoldWhite   "${Esc}[1;37m")
endif()

set(MONGOOSE_LIB_FILES
        Include/mongoose.hpp
        Include/mongoose_boundaryheap.hpp
        Include/mongoose_coarsening.hpp
        Include/mongoose_conditioning.hpp
        Include/mongoose_cs.hpp
        Include/mongoose_cutcost.hpp
        Include/mongoose_debug.hpp
        Include/mongoose_fiducciamattheyses.hpp
        Include/mongoose_gradproj.hpp
        Include/mongoose_graph.hpp
        Include/mongoose_guesscut.hpp
        Include/mongoose_hager.hpp
        Include/mongoose_internal.hpp
        Include/mongoose_interop.hpp
        Include/mongoose_io.hpp
        Include/mongoose_matching.hpp
        Include/mongoose_napsack.hpp
        Include/mongoose_options.hpp
        Include/mongoose_qpdelta.hpp
        Include/mongoose_refining.hpp
        Include/mongoose_waterdance.hpp
        Source/mongoose_boundaryheap.cpp
        Source/mongoose_coarsening.cpp
        Source/mongoose_conditioning.cpp
        Source/mongoose_cs.cpp
        Source/mongoose_debug.cpp
        Source/mongoose_edgeseparator.cpp
        Source/mongoose_fiducciamattheyses.cpp
        Source/mongoose_guesscut.cpp
        Source/mongoose_internal.cpp
        Source/mongoose_interop.cpp
        Source/mongoose_io.cpp
        Source/mongoose_matching.cpp
        Source/mongoose_nuri_qpgradproj.cpp
        Source/mongoose_qpboundary.cpp
        Source/mongoose_qpgradproj.cpp
        Source/mongoose_qplinks.cpp
        Source/mongoose_qpmaxheap.cpp
        Source/mongoose_qpminheap.cpp
        Source/mongoose_qpnapdown.cpp
        Source/mongoose_qpnapsack.cpp
        Source/mongoose_qpnapsack2.cpp
        Source/mongoose_qpnapup.cpp
        Source/mongoose_refinement.cpp
        Source/mongoose_waterdance.cpp
        External/Include/mmio.h
        External/Source/mmio.c)

set(EXE_FILES
    Executable/Mongoose.cpp)

set(DEMO_FILES
    Demo/Demo.cpp)

set(TEST_FILES
    Tests/mongoose_test_io.cpp)

# Specify the Include directory for *.hpp files
include_directories(Include
                    External/Include)

# Build the Mongoose library
add_library(MongooseLib ${MONGOOSE_LIB_FILES})
set_property(TARGET MongooseLib PROPERTY CXX_STANDARD 11)

# Build the Mongoose executable
add_executable(Mongoose ${EXE_FILES})
target_link_libraries(Mongoose MongooseLib)

# Build the Demo executable
add_executable(Demo ${DEMO_FILES})
target_link_libraries(Demo MongooseLib)

# Build the Test executable
enable_testing()

add_executable(mongoose_test_io ${TEST_FILES})
target_link_libraries(mongoose_test_io MongooseLib)

add_test( TEST_IO mongoose_test_io )

# Coverage Tests
# Build the Mongoose library
add_library(MongooseLibCov ${MONGOOSE_LIB_FILES})
add_executable(mongoose_test_io_coverage ${TEST_FILES})

MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER_ID} )
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # using Clang
  SET(CMAKE_CXX_FLAGS_COVERAGE "--coverage")
  SET(CMAKE_C_FLAGS_COVERAGE "--coverage")
  SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE "--coverage")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  SET(CMAKE_CXX_FLAGS_COVERAGE "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS_COVERAGE "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE "-fprofile-arcs -ftest-coverage")
#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

target_link_libraries(mongoose_test_io_coverage MongooseLibCov)
set_target_properties(MongooseLibCov PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_COVERAGE}")
set_target_properties(MongooseLibCov PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")

set_target_properties(mongoose_test_io_coverage PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_COVERAGE}")
set_target_properties(mongoose_test_io_coverage PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_COVERAGE}")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
add_test(Coverage mongoose_test_io_coverage)
#get_property(mongoose_lib_location TARGET mongoose_test_io_coverage PROPERTY LOCATION)

#add_custom_command(TARGET mongoose_test_io_coverage
#                   POST_BUILD
#                   COMMAND mongoose_test_io_coverage && gcov ../Source/*.cpp -o ./CMakeFiles/MongooseLibCov.dir/Source/*.o)
