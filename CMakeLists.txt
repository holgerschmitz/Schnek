cmake_minimum_required(VERSION 3.25.2)

set(SCHNEK_VERSION 1.2.0)

# set the project name
project(Schnek VERSION ${SCHNEK_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(MPI_CXX_SKIP_MPICXX TRUE)

find_package(MPI COMPONENTS CXX)
find_package(HDF5 COMPONENTS CXX)
find_package(Kokkos CONFIG)
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
    src/decomposition/mpi_context.cpp
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

target_include_directories(schnek
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(schnek PUBLIC cxx_std_17)

target_compile_options(schnek PRIVATE
  -Wall -Wextra
)

target_link_libraries(schnek
  PUBLIC
    Boost::boost
)

if(MPI_CXX_FOUND)
  target_link_libraries(schnek PUBLIC MPI::MPI_CXX)
endif()

if(HDF5_FOUND)
  target_link_libraries(schnek PUBLIC HDF5::HDF5)
endif()

function(schnek_mark_kokkos_target target_name)
  if(Kokkos_FOUND AND Kokkos_COMPILE_LANGUAGE)
    set_property(TARGET ${target_name} PROPERTY
      ${Kokkos_COMPILE_LANGUAGE}_ARCHITECTURES
      ${Kokkos_${Kokkos_COMPILE_LANGUAGE}_ARCHITECTURES}
    )

    set_property(TARGET ${target_name} PROPERTY
      ${Kokkos_COMPILE_LANGUAGE}_STANDARD
      ${Kokkos_${Kokkos_COMPILE_LANGUAGE}_STANDARD}
    )
  endif()
endfunction()

if(Kokkos_FOUND)
  message(STATUS "Kokkos_VERSION = ${Kokkos_VERSION}")
  message(STATUS "Kokkos_DIR = ${Kokkos_DIR}")
  message(STATUS "Kokkos_COMPILE_LANGUAGE = ${Kokkos_COMPILE_LANGUAGE}")
  message(STATUS "Kokkos_CUDA_ARCHITECTURES = ${Kokkos_CUDA_ARCHITECTURES}")
  message(STATUS "Kokkos_CUDA_STANDARD = ${Kokkos_CUDA_STANDARD}")
  message(STATUS "CMAKE_CUDA_ARCHITECTURES = ${CMAKE_CUDA_ARCHITECTURES}")

  target_link_libraries(schnek PUBLIC Kokkos::kokkos)
  target_compile_definitions(schnek PUBLIC KOKKOS_DEPENDENCE)

  message(STATUS "Kokkos_COMPILE_LANGUAGE = ${Kokkos_COMPILE_LANGUAGE}")

  if(Kokkos_COMPILE_LANGUAGE)
    enable_language(${Kokkos_COMPILE_LANGUAGE})
  endif()
  schnek_mark_kokkos_target(schnek)
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
    testsuite/decomposition/test_domaindecomposition.cpp
    testsuite/decomposition/mpi_test_context.cpp
    testsuite/decomposition/test_mpi_cartesian_decomposition.cpp
    testsuite/decomposition/test_kokkos_exchange.cpp
    testsuite/decomposition/test_for_each_cell.cpp
    testsuite/decomposition/test_serial_decomposition.cpp
    testsuite/decomposition/detail/test_grid_factory.cpp
    testsuite/decomposition/detail/test_grid_transfer.cpp
    testsuite/decomposition/detail/test_grid_visitor.cpp
    testsuite/computation/test_algorithm.cpp
    testsuite/diagnostic/test_hdfdiagnostic.cpp
    testsuite/generic/test_typelist.cpp
    testsuite/generic/test_static_range.cpp
    testsuite/grid/test_c_storage.cpp
    testsuite/grid/test_fortran_storage.cpp
    testsuite/grid/test_kokkos_storage.cpp
    testsuite/grid/test_range_c_iteration.cpp
    testsuite/grid/test_range_fortran_iteration.cpp
    testsuite/grid/test_range_kokkos_iteration.cpp
    testsuite/grid/concepts/test_concepts.cpp
)

if(Kokkos_FOUND)
  set_source_files_properties(
      testsuite/main.cpp
      testsuite/utility.cpp
      testsuite/decomposition/test_domaindecomposition.cpp
      testsuite/decomposition/test_for_each_cell.cpp
      testsuite/decomposition/test_kokkos_exchange.cpp
      testsuite/decomposition/test_mpi_cartesian_decomposition.cpp
      testsuite/decomposition/test_serial_decomposition.cpp
      testsuite/decomposition/detail/test_grid_transfer.cpp
      testsuite/diagnostic/test_hdfdiagnostic.cpp
      testsuite/grid/test_kokkos_storage.cpp
      testsuite/grid/test_range_kokkos_iteration.cpp
      PROPERTIES LANGUAGE ${Kokkos_COMPILE_LANGUAGE}
  )
  
  schnek_mark_kokkos_target(schnek_tests)
endif()

target_link_libraries(schnek_tests PRIVATE schnek)
target_compile_features(schnek_tests PRIVATE cxx_std_17)

add_test(NAME test COMMAND schnek_tests)
enable_testing()

