cmake_minimum_required(VERSION 3.2.2)
include(CMake/cmake_colors.cmake)

project(Mongoose)

set(MONGOOSE_VERSION_MAJOR   "1")
set(MONGOOSE_VERSION_MINOR   "0")
set(MONGOOSE_VERSION_PATCH   "4")
set(MONGOOSE_VERSION "${MONGOOSE_VERSION_MAJOR}.
                      ${MONGOOSE_VERSION_MINOR}.
                      ${MONGOOSE_VERSION_PATCH}")

set(MONGOOSE_FILES
    Include/Mongoose_Coarsening.hpp
    Include/Mongoose_Conditioning.hpp
    Include/Mongoose_CSparse.hpp
    Include/Mongoose_CutCost.hpp
    Include/Mongoose_Debug.hpp
    Include/Mongoose_EdgeSeparator.hpp
    Include/Mongoose_Graph.hpp
    Include/Mongoose_GuessCut.hpp
    Include/Mongoose_ImproveFM.hpp
    Include/Mongoose_ImproveQP.hpp
    Include/Mongoose_Internal.hpp
    Include/Mongoose_Interop.hpp
    Include/Mongoose_IO.hpp
    Include/Mongoose_Logger.hpp
    Include/Mongoose_Matching.hpp
    Include/Mongoose_Options.hpp
    Include/Mongoose_Refinement.hpp
    Include/Mongoose_Sanitize.hpp
    Include/Mongoose_Waterdance.hpp
    Source/Mongoose_Coarsening.cpp
    Source/Mongoose_Conditioning.cpp
    Source/Mongoose_CSparse.cpp
    Source/Mongoose_Debug.cpp
    Source/Mongoose_EdgeSeparator.cpp
    Source/Mongoose_Graph.cpp
    Source/Mongoose_GuessCut.cpp
    Source/Mongoose_ImproveFM.cpp
    Source/Mongoose_ImproveQP.cpp
    Source/Mongoose_Interop.cpp
    Source/Mongoose_IO.cpp
    Source/Mongoose_Logger.cpp
    Source/Mongoose_Matching.cpp
    Source/Mongoose_Options.cpp
    Source/Mongoose_Refinement.cpp
    Source/Mongoose_Sanitize.cpp
    Source/Mongoose_Waterdance.cpp
  )

set(NAPHEAP_FILES
    Include/Mongoose_QPBoundary.hpp
    Include/Mongoose_QPBoundaryHeap.hpp
    Include/Mongoose_QPDelta.hpp
    Include/Mongoose_QPGradProj.hpp
    Include/Mongoose_QPLinks.hpp
    Include/Mongoose_QPMaxHeap.hpp
    Include/Mongoose_QPMinHeap.hpp
    Include/Mongoose_QPNapDown.hpp
    Include/Mongoose_QPNapsack.hpp
    Include/Mongoose_QPNapsackPair.hpp
    Include/Mongoose_QPNapUp.hpp
    Source/Mongoose_QPBoundary.cpp
    Source/Mongoose_QPBoundaryHeap.cpp
    Source/Mongoose_QPDelta.cpp
    Source/Mongoose_QPGradProj.cpp
    Source/Mongoose_QPLinks.cpp
    Source/Mongoose_QPMaxHeap.cpp
    Source/Mongoose_QPMinHeap.cpp
    Source/Mongoose_QPNapDown.cpp
    Source/Mongoose_QPNapsack.cpp
    Source/Mongoose_QPNapsackPair.cpp
    Source/Mongoose_QPNapUp.cpp
  )

set(MMIO_FILES
    External/mmio/Source/mmio.c
    External/mmio/Include/mmio.h)

set(MONGOOSE_LIB_FILES
    ${MONGOOSE_FILES}
    ${NAPHEAP_FILES}
    ${MMIO_FILES})

set(EXE_FILES
    Executable/Mongoose.cpp)

set(DEMO_FILES
    Demo/Demo.cpp)

# Specify the Include directory for *.hpp files
include_directories(Include
                    External/mmio/Include
                    )

# Require C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Look for common SuiteSparse_config - otherwise, use our own
set(SUITESPARSE_CONFIG_INCLUDE_DIRS
  ${SUITESPARSE_CONFIG_INCLUDE_DIR_HINT}
  /usr/local/include
  /usr/include
  /include
  /opt/include
)

set(SUITESPARSE_CONFIG_LIBRARY_DIRS
  ${SUITESPARSE_CONFIG_LIBRARY_DIR_HINT}
  /usr/local/lib
  /usr/lib
  /lib
  /opt/lib
)

find_path(SUITESPARSE_CONFIG_INCLUDE_DIR
  NAMES SuiteSparse_config.h
  PATHS ${SUITESPARSE_CONFIG_INCLUDE_DIRS}
)

find_library(SUITESPARSE_CONFIG_LIBRARY
  NAMES libsuitesparseconfig.a libsuitesparseconfig.dylib libsuitesparseconfig.so
  PATHS ${SUITESPARSE_CONFIG_LIBRARY_DIRS}
)

if(EXISTS ${SUITESPARSE_CONFIG_INCLUDE_DIR} AND EXISTS ${SUITESPARSE_CONFIG_LIBRARY})
  message( STATUS "Installed SuiteSparse_config" ${BoldGreen} " found" ${ColourReset} " in " ${SUITESPARSE_CONFIG_INCLUDE_DIR} " and " ${SUITESPARSE_CONFIG_LIBRARY})
  include_directories(${SUITESPARSE_CONFIG_INCLUDE_DIR})
elseif(EXISTS ${PROJECT_SOURCE_DIR}/../SuiteSparse_config)
  set(SUITESPARSE_CONFIG_DIR ${PROJECT_SOURCE_DIR}/../SuiteSparse_config)
  include_directories(${SUITESPARSE_CONFIG_DIR})

  if(EXISTS ${SUITESPARSE_CONFIG_DIR}/libsuitesparseconfig.a)
    set(SUITESPARSE_CONFIG_LIBRARY ${SUITESPARSE_CONFIG_DIR}/libsuitesparseconfig.a)
    message( STATUS "External SuiteSparse_config" ${BoldGreen} " found already compiled" ${ColourReset} ", compiling as a SuiteSparse module.")
  else()
    message( STATUS "External SuiteSparse_config" ${BoldGreen} " found " ${BoldRed} "uncompiled" ${ColourReset} ", compiling as a SuiteSparse module.")
    set(SUITESPARSE_CONFIG_FILES
    ${SUITESPARSE_CONFIG_DIR}/SuiteSparse_config.c
    ${SUITESPARSE_CONFIG_DIR}/SuiteSparse_config.h
  )
  
  set(MONGOOSE_LIB_FILES
    ${MONGOOSE_LIB_FILES}
    ${SUITESPARSE_CONFIG_FILES}
  )
  endif()
else()
  message( STATUS "External SuiteSparse_config" ${BoldRed} " not found" ${ColourReset} ", compiling stand-alone Mongoose.")
  set(SUITESPARSE_CONFIG_DIR ${PROJECT_SOURCE_DIR}/SuiteSparse_config)
  set(SUITESPARSE_CONFIG_FILES
    ${SUITESPARSE_CONFIG_DIR}/SuiteSparse_config.c
    ${SUITESPARSE_CONFIG_DIR}/SuiteSparse_config.h
  )
  include_directories(${SUITESPARSE_CONFIG_DIR})
  set(MONGOOSE_LIB_FILES
    ${MONGOOSE_LIB_FILES}
    ${SUITESPARSE_CONFIG_FILES}
  )
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build the Mongoose library
add_library(mongoose ${MONGOOSE_LIB_FILES})
if(EXISTS ${SUITESPARSE_CONFIG_LIBRARY})
  target_link_libraries(mongoose ${SUITESPARSE_CONFIG_LIBRARY})
endif()

# Build the Mongoose library and all tests with coverage flags
add_library(mongoose_dbg ${MONGOOSE_LIB_FILES})
if(EXISTS ${SUITESPARSE_CONFIG_LIBRARY})
  target_link_libraries(mongoose_dbg ${SUITESPARSE_CONFIG_LIBRARY})
endif()

# Build the Mongoose executable
add_executable(mongoose_exe ${EXE_FILES})
set_target_properties(mongoose_exe
  PROPERTIES OUTPUT_NAME mongoose)
target_link_libraries(mongoose_exe mongoose)

# Build the Demo executable
add_executable(Demo ${DEMO_FILES})
target_link_libraries(Demo mongoose)

# Coverage and Unit Testing Setup
enable_testing()
set(TESTING_OUTPUT_PATH ${CMAKE_BINARY_DIR}/tests)

# I/O Tests
add_executable(mongoose_test_io 
  Tests/Mongoose_Test_IO.cpp
  Tests/Mongoose_Test_IO_exe.cpp)
target_link_libraries(mongoose_test_io mongoose_dbg)
set_target_properties(mongoose_test_io PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})

add_test(IO_Test ./runTests.py -min 1 -max 15 -t io -k)

# Edge Separator Tests
add_executable(mongoose_test_edgesep 
  Tests/Mongoose_Test_EdgeSeparator.cpp
  Tests/Mongoose_Test_EdgeSeparator_exe.cpp)
target_link_libraries(mongoose_test_edgesep mongoose_dbg)
set_target_properties(mongoose_test_edgesep PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})

add_test(Edge_Separator_Test ./runTests.py -min 1 -max 15 -t edgesep)
add_test(Edge_Separator_Test_2 ./runTests.py -t edgesep -i 21 39 1557 1562 353 2468 1470 1380 505 182 201 2331 760 1389 2401 2420 242 250 1530 1533)

# Memory Tests
add_executable(mongoose_test_memory 
  Tests/Mongoose_Test_Memory.cpp
  Tests/Mongoose_Test_Memory_exe.cpp)
target_link_libraries(mongoose_test_memory mongoose_dbg)
set_target_properties(mongoose_test_memory PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})

add_test(Memory_Test ./runTests.py -min 1 -max 15 -t memory)

# Performance Test
add_executable(mongoose_test_performance
  Tests/Mongoose_Test_Performance.cpp
  Tests/Mongoose_Test_Performance_exe.cpp)
target_link_libraries(mongoose_test_performance mongoose)
set_target_properties(mongoose_test_performance PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})
add_test(Performance_Test ./runTests.py -min 1 -max 15 -t performance -p)
add_test(Performance_Test_2 ./runTests.py -t performance -i 21 39 1557 1562 353 2468 1470 1380 505 182 201 2331 760 1389 2401 2420 242 250 1530 1533 -p)

# Reference Test
add_executable(mongoose_test_reference
  Tests/Mongoose_Test_Reference.cpp
  Tests/Mongoose_Test_Reference_exe.cpp)
target_link_libraries(mongoose_test_reference mongoose)
set_target_properties(mongoose_test_reference PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})

# Unit Tests
add_executable(mongoose_unit_test_io
  Tests/Mongoose_UnitTest_IO_exe.cpp)
target_link_libraries(mongoose_unit_test_io mongoose_dbg)
set_target_properties(mongoose_unit_test_io PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})
add_test(Unit_Test_IO ./tests/mongoose_unit_test_io)

add_executable(mongoose_unit_test_graph
  Tests/Mongoose_UnitTest_Graph_exe.cpp)
target_link_libraries(mongoose_unit_test_graph mongoose_dbg)
set_target_properties(mongoose_unit_test_graph PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${TESTING_OUTPUT_PATH})
add_test(Unit_Test_Graph ./tests/mongoose_unit_test_graph)

message( STATUS "CMAKE_CXX_COMPILER: " ${BoldBlue} ${CMAKE_CXX_COMPILER_ID} ${ColourReset})
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  # using Clang
  SET(CMAKE_CXX_FLAGS_DEBUG "--coverage -g")
  SET(CMAKE_C_FLAGS_DEBUG "--coverage -g")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "--coverage -g")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  # using GCC
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS_DEBUG "-g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
  SET(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fprofile-arcs -ftest-coverage")
#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
  # using Intel C++
#elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  # using Visual Studio C++
endif()

set_target_properties(mongoose_dbg PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_dbg PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

# Add debug compile/linker flags
set_target_properties(mongoose_test_io PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_test_io PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set_target_properties(mongoose_test_memory PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_test_memory PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set_target_properties(mongoose_test_edgesep PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_test_edgesep PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set_target_properties(mongoose_unit_test_io PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_unit_test_io PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set_target_properties(mongoose_unit_test_graph PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_unit_test_graph PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1) # Necessary for gcov - prevents file.cpp.gcda instead of file.gcda

# Copy over runTest.py to build folder for ease of use
file(COPY Tests/runTests.py DESTINATION ${CMAKE_BINARY_DIR})
file(COPY Tests/runCompare.py DESTINATION ${CMAKE_BINARY_DIR})

add_custom_target(purge
    COMMAND rm -rf ${CMAKE_BINARY_DIR}/*
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
