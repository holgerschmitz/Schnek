cmake_minimum_required(VERSION 3.10)

set(SCHNEK_VERSION 1.2.0)

enable_language(CXX)

# set the project name
project(Schnek VERSION ${SCHNEK_VERSION})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(MPI)
find_package(HDF5)
find_package(Kokkos PATHS ${KOKKOS_DIR})
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
target_include_directories(schnek PUBLIC ${Boost_INCLUDE_DIRS})

target_link_libraries(schnek PUBLIC ${MPI_C_LIBRARIES})
target_link_libraries(schnek PUBLIC ${HDF5_LIBRARIES})


if (Kokkos_FOUND)
  target_include_directories(schnek PUBLIC ${Kokkos_INCLUDE_DIR})
  target_compile_definitions(schnek PUBLIC KOKKOS_DEPENDENCE)
  target_link_libraries(schnek PUBLIC Kokkos::kokkoscore)
endif()

# Uncomment to see all defined variables

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

set(SCHNEK_HAVE_HDF5 ${HDF5_FOUND})
set(SCHNEK_HAVE_MPI ${MPI_FOUND})
set(SCHNEK_USE_HDF_PARALLEL ${HDF5_IS_PARALLEL})
set(SCHNEK_HAVE_KOKKOS ${Kokkos_FOUND})

set(include_dest_root "include")
set(include_dest "${include_dest_root}/schnek")
set(lib_dest_root "lib")
set(lib_dest "${lib_dest_root}/schnek")

configure_file(src/config.hpp.in src/config.hpp)


install(TARGETS schnek 
  EXPORT schnek 
  LIBRARY DESTINATION "${lib_dest_root}"
  INCLUDES DESTINATION "${include_dest_root}")
install(DIRECTORY src/ DESTINATION "${include_dest}" FILES_MATCHING PATTERN "*.hpp")
install(DIRECTORY src/ DESTINATION "${include_dest}" FILES_MATCHING PATTERN "*.t")
install(EXPORT schnek DESTINATION "${lib_dest}")
install(FILES schnek-config.cmake DESTINATION "${lib_dest_root}/schnek")

###########################################################
# Unit Tests

include(CTest)

# Add test executable
add_executable (schnek_tests EXCLUDE_FROM_ALL
    testsuite/main.cpp
    testsuite/test_array.cpp
    testsuite/test_arrayexpression.cpp
    testsuite/test_parser.cpp
    testsuite/test_range.cpp
    testsuite/utility.cpp
    testsuite/computation/test_algorithm.cpp
    testsuite/generic/test_typelist.cpp
    testsuite/generic/test_static_range.cpp
    testsuite/grid/test_c_storage.cpp
    testsuite/grid/test_fortran_storage.cpp
    testsuite/grid/test_kokkos_storage.cpp
    testsuite/grid/test_range_c_iteration.cpp
    testsuite/grid/test_range_fortran_iteration.cpp
    testsuite/grid/test_range_kokkos_iteration.cpp
)

target_include_directories(schnek_tests PUBLIC "src")
target_include_directories(schnek_tests PUBLIC ${MPI_INCLUDE_PATH})
target_include_directories(schnek_tests PUBLIC ${HDF5_INCLUDE_DIRS})
target_include_directories(schnek_tests PUBLIC ${Boost_INCLUDE_DIRS})

if (Kokkos_FOUND)
  target_include_directories(schnek_tests PUBLIC ${Kokkos_INCLUDE_DIR})
  target_compile_definitions(schnek_tests PUBLIC KOKKOS_DEPENDENCE)
  target_link_libraries(schnek PUBLIC Kokkos::kokkoscore)
endif()

target_link_libraries(schnek_tests schnek)
target_link_libraries(schnek_tests ${MPI_C_LIBRARIES})
target_link_libraries(schnek_tests ${HDF5_LIBRARIES})

target_compile_features(schnek_tests PRIVATE cxx_std_14)

add_test(NAME test COMMAND schnek_tests)
enable_testing()