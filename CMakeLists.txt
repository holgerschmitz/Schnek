cmake_minimum_required(VERSION 3.10)

set(SCHNEK_VERSION 1.2.0)

enable_language(CXX)

# set the project name
project(Schnek VERSION ${SCHNEK_VERSION})

find_package(MPI)
find_package(HDF5)
find_package(Boost REQUIRED)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# add the library
add_library(schnek SHARED
    src/diagnostic/diagnostic.cpp
    src/diagnostic/hdfdiagnostic.cpp
    src/functions.cpp
    src/grid/mpisubdivision.cpp
    src/parser/deckscanner.cpp
    src/parser/parser.cpp
    src/parser/parsertoken.cpp
    src/tools/literature.cpp
    src/util/exceptions.cpp
    src/util/factor.cpp
    src/variables/blockclasses.cpp
    src/variables/block.cpp
    src/variables/blockparameters.cpp
    src/variables/dependencies.cpp
    src/variables/function_expression.cpp
    src/variables/variables.cpp
)

target_include_directories(schnek PUBLIC ${MPI_INCLUDE_PATH})
target_include_directories(schnek PUBLIC ${HDF5_INCLUDE_DIRS})

target_link_libraries(schnek ${MPI_C_LIBRARIES})
target_link_libraries(schnek ${HDF5_LIBRARIES})

set(HAVE_BOOST ${Boost_FOUND})
set(HAVE_HDF5 ${HDF5_FOUND})
set(SCHNEK_HAVE_MPI ${MPI_FOUND})
set(SCHNEK_USE_HDF_PARALLEL ${HDF5_IS_PARALLEL})
configure_file(src/config.hpp.in src/config.hpp)

install(TARGETS schnek LIBRARY DESTINATION lib)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

###########################################################
# Unit Tests

include(CTest)

# Add test executable
add_executable (schnek_tests EXCLUDE_FROM_ALL
    testsuite/main.cpp
    testsuite/test_array.cpp
    testsuite/test_arrayexpression.cpp
    testsuite/test_grid.cpp
    testsuite/test_parser.cpp
    testsuite/test_range.cpp
    testsuite/utility.cpp
)
target_link_libraries(schnek_tests schnek)
target_include_directories(schnek_tests PUBLIC "src")

add_test(NAME test COMMAND schnek_tests)
enable_testing()