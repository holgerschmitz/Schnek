find_package(PkgConfig)
pkg_check_modules(PC_Kokkos QUIET kokkos)

find_path(Kokkos_INCLUDE_DIR
  NAMES Kokkos_Core.hpp
  PATHS ${PC_Kokkos_INCLUDE_DIRS}
)

find_library(Kokkos_LIBRARY
  NAMES kokkoscore
  PATHS ${PC_Kokkos_LIBRARY_DIRS}
)

set(Kokkos_VERSION ${PC_Kokkos_VERSION})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Kokkos
  FOUND_VAR Kokkos_FOUND
  REQUIRED_VARS
    Kokkos_LIBRARY
    Kokkos_INCLUDE_DIR
  VERSION_VAR Kokkos_VERSION 
)

if(Kokkos_FOUND AND NOT TARGET Kokkos::kokkos)
  add_library(Kokkos::kokkos UNKNOWN IMPORTED)  
  set_target_properties(Kokkos::kokkos PROPERTIES
    IMPORTED_LOCATION "${Kokkos_LIBRARY}"
    INTERFACE_COMPILE_OPTIONS "${PC_Kokkos_CFLAGS_OTHER}"
    INTERFACE_INCLUDE_DIRECTORIES "${Kokkos_INCLUDE_DIR}"
  )
endif()