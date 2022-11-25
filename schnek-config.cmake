get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
include(${SELF_DIR}/schnek.cmake)

# set(Schnek_INCLUDE $<INSTALL_INTERFACE:${include_dest}>)

# target_include_directories(schnek PUBLIC
# 											 $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # for config_impl.hpp when building
#                        $<INSTALL_INTERFACE:${include_dest}>  # for client in install mode
#                        $<INSTALL_INTERFACE:${main_lib_dest}> # for config_impl.hpp in install mode
#                        )
