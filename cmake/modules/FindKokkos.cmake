# Find the Kokkos library

set(KOKKOS_PATH "" CACHE STRING "The path to the Kokkos library")

if("${KOKKOS_PATH}" STREQUAL "")
  set(KOKKOS_PATH "$ENV{KOKKOS_PATH}")
endif()

if (NOT "${KOKKOS_PATH}" STREQUAL "")
  message(STATUS "Kokkos: Looking for Kokkos in ${KOKKOS_PATH}")
endif()

find_path(KOKKOS_INCLUDE_DIR Kokkos_Core.hpp PATHS "${KOKKOS_PATH}/include")
find_library(KOKKOS_CORE_LIBRARY NAMES kokkoscore PATHS "${KOKKOS_PATH}/lib")

if (NOT ("${KOKKOS_INCLUDE_DIR}" STREQUAL "KOKKOS_INCLUDE_DIR-NOTFOUND"
      OR "${KOKKOS_CORE_LIBRARY}" STREQUAL "KOKKOS_CORE_LIBRARY-NOTFOUND"))
  set(Kokkos_LIBRARIES "${KOKKOS_CORE_LIBRARY}")
  set(Kokkos_INCLUDE_DIRS "${KOKKOS_INCLUDE_DIR}")
  set(Kokkos_FOUND TRUE)
  message(STATUS "Found Kokkos: ${Kokkos_LIBRARIES}")
endif()

