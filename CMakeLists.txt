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
        Include/mongoose.hpp
        Include/mongoose_coarsening.hpp
        Include/mongoose_conditioning.hpp
        Include/mongoose_cs.hpp
        Include/mongoose_cutcost.hpp
        Include/mongoose_debug.hpp
        Include/mongoose_fiducciamattheyses.hpp
        Include/mongoose_graph.hpp
        Include/mongoose_guesscut.hpp
        Include/mongoose_hager.hpp
        Include/mongoose_internal.hpp
        Include/mongoose_interop.hpp
        Include/mongoose_io.hpp
        Include/mongoose_matching.hpp
        Include/mongoose_options.hpp
        Include/mongoose_refining.hpp
        Include/mongoose_sanitize.hpp
        Include/mongoose_waterdance.hpp
        Source/mongoose_boundaryheap.cpp
        Source/mongoose_coarsening.cpp
        Source/mongoose_conditioning.cpp
        Source/mongoose_cs.cpp
        Source/mongoose_debug.cpp
        Source/mongoose_edgeseparator.cpp
        Source/mongoose_fiducciamattheyses.cpp
        Source/mongoose_guesscut.cpp
        Source/mongoose_interop.cpp
        Source/mongoose_io.cpp
        Source/mongoose_matching.cpp
        Source/mongoose_nuri_qpgradproj.cpp
        Source/mongoose_refinement.cpp
        Source/mongoose_sanitize.cpp
        Source/mongoose_waterdance.cpp
      )

set(NAPHEAP_FILES
    Source/mongoose_qpboundary.cpp
    Source/mongoose_qpgradproj.cpp
    Source/mongoose_qplinks.cpp
    Source/mongoose_qpmaxheap.cpp
    Source/mongoose_qpminheap.cpp
    Source/mongoose_qpnapdown.cpp
    Source/mongoose_qpnapsack.cpp
    Source/mongoose_qpnapsack2.cpp
    Source/mongoose_qpnapup.cpp
    Include/mongoose_boundaryheap.hpp
    Include/mongoose_napsack.hpp
    Include/mongoose_qpdelta.hpp
    Include/mongoose_gradproj.hpp
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

set(ALL_TEST_FILES
    Tests/mongoose_test.hpp
    Tests/mongoose_test_io.cpp
    Tests/mongoose_test_memory.cpp
    Tests/mongoose_run_all_tests.cpp)

# Specify the Include directory for *.hpp files
include_directories(Include
                    External/mmio/Include
                    External/napheap/Include)

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
SET_TARGET_PROPERTIES(mongoose_exe
  PROPERTIES OUTPUT_NAME mongoose)
target_link_libraries(mongoose_exe mongoose)

# Build the Demo executable
add_executable(Demo ${DEMO_FILES})
target_link_libraries(Demo mongoose)

# Coverage and Unit Testing Setup
enable_testing()

# Unit Tests
add_executable(mongoose_test_io 
  Tests/mongoose_test_io.cpp
  Tests/mongoose_test_io_exe.cpp)
target_link_libraries(mongoose_test_io mongoose_dbg)

add_test(test_io mongoose_test_io)

# Exhaustive Memory Test
add_executable(mongoose_test_memory 
  Tests/mongoose_test_memory.cpp
  Tests/mongoose_test_memory_exe.cpp)
target_link_libraries(mongoose_test_memory mongoose_dbg)

# Timing and Results Test
add_executable(mongoose_test_results
  Tests/mongoose_test_results.cpp
  Tests/mongoose_test_results_exe.cpp)
target_link_libraries(mongoose_test_results mongoose)

#add_test(test_memory mongoose_memory_test)

# Coverage Tests

add_executable(mongoose_run_all_tests ${ALL_TEST_FILES})

MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${BoldBlue} ${CMAKE_CXX_COMPILER_ID} ${ColourReset})
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

target_link_libraries(mongoose_run_all_tests mongoose_dbg)
set_target_properties(mongoose_dbg PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_dbg PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

# Temporary
set_target_properties(mongoose_test_io PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_test_io PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set_target_properties(mongoose_test_memory PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_test_memory PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")

set_target_properties(mongoose_run_all_tests PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS_DEBUG}")
set_target_properties(mongoose_run_all_tests PROPERTIES LINK_FLAGS "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
set(CMAKE_CXX_OUTPUT_EXTENSION_REPLACE 1)
add_test(Coverage mongoose_run_all_tests)
