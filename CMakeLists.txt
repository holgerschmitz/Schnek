cmake_minimum_required(VERSION 3.10)

# set the project name
project(Schnek)

cmake_policy(SET CMP0076 NEW)

find_package(MPI)
find_package(HDF5)

# add the library
add_library(schnek SHARED)

target_include_directories(schnek ${MPI_INCLUDE_PATH})
target_include_directories(schnek ${HDF5_INCLUDE_DIRS})

target_link_libraries(schnek ${MPI_C_LIBRARIES})
target_link_libraries(schnek ${HDF5_LIBRARIES})

message("MPI ${MPI_INCLUDE_PATH} | ${MPI_C_LIBRARIES}")
message("HDF5 ${HDF5_INCLUDE_DIRS} | ${HDF5_LIBRARIES}")

add_subdirectory(src)
