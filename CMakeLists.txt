cmake_minimum_required(VERSION 3.3)
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
        Source/mongoose_edgeseparator.cpp
        Source/mongoose_fiducciamattheyses.cpp
        Source/mongoose_guesscut.cpp
        Source/mongoose_internal.cpp
        Source/mongoose_interop.cpp
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
        Source/mongoose_waterdance.cpp)

set(DEMO_FILES
    Demo/Demo.cpp)

set(TEST_FILES
    Tcov/test1.cpp)

# Specify the Include directory for *.hpp files
include_directories(Include)

# Build the Mongoose library
add_library(Mongoose ${MONGOOSE_LIB_FILES})
set_property(TARGET Mongoose PROPERTY CXX_STANDARD 11)

# Build the Demo executable
add_executable(Demo ${DEMO_FILES})
target_link_libraries(Demo Mongoose)

# Build the test executable
add_executable(test1 ${TEST_FILES})
target_link_libraries(test1 Mongoose)