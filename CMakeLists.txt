cmake_minimum_required(VERSION 3.9)
project(s7g2_sdk VERSION 1.0.0 LANGUAGES C CXX ASM)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

include(${CMAKE_CURRENT_BINARY_DIR}/conan_paths.cmake)

# Device SDK
add_library(s7g2_sdk STATIC)
add_library(s7g2::s7g2_sdk ALIAS s7g2_sdk)
target_sources(s7g2_sdk PRIVATE
    synergy/ssp/src/driver/r_ioport/r_ioport.c
    synergy/ssp/src/driver/r_fmi/r_fmi.c
    synergy/ssp/src/driver/r_elc/r_elc.c
    synergy/ssp/src/driver/r_cgc/hw/hw_cgc.c
    synergy/ssp/src/driver/r_cgc/r_cgc.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_cache.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_clocks.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_feature.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_fmi_R7FS7G27H3A01CFC.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_group_irq.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_hw_locks.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_module_stop.c
    synergy/ssp/src/bsp/mcu/s7g2/bsp_rom_registers.c
    synergy/ssp/src/bsp/mcu/all/bsp_common.c
    synergy/ssp/src/bsp/mcu/all/bsp_common_leds.c
    synergy/ssp/src/bsp/mcu/all/bsp_delay.c
    synergy/ssp/src/bsp/mcu/all/bsp_irq.c
    synergy/ssp/src/bsp/mcu/all/bsp_locking.c
    synergy/ssp/src/bsp/mcu/all/bsp_register_protection.c
    synergy/ssp/src/bsp/mcu/all/bsp_sbrk.c
    synergy/ssp/src/bsp/cmsis/Device/RENESAS/S7G2/Source/startup_S7G2.c
    synergy/ssp/src/bsp/cmsis/Device/RENESAS/S7G2/Source/system_S7G2.c
    synergy/board/s7g2_sk/bsp_init.c
    synergy/board/s7g2_sk/bsp_leds.c
    synergy/board/s7g2_sk/bsp_qspi.c
    synergy_gen/common_data.c
    synergy_gen/hal_data.c
    synergy_gen/pin_data.c
)

# Header locations
target_include_directories(s7g2_sdk
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy_gen>
    $<INSTALL_INTERFACE:include/synergy_gen>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy_cfg/ssp_cfg/bsp>
    $<INSTALL_INTERFACE:include/synergy_cfg/ssp_cfg/bsp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy_cfg/ssp_cfg/driver>
    $<INSTALL_INTERFACE:include/synergy_cfg/ssp_cfg/driver>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy/ssp/inc>
    $<INSTALL_INTERFACE:include/synergy/ssp/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy/ssp/inc/bsp>
    $<INSTALL_INTERFACE:include/synergy/ssp/inc/bsp>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy/ssp/inc/bsp/cmsis/Include>
    $<INSTALL_INTERFACE:include/synergy/ssp/inc/bsp/cmsis/Include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy/ssp/inc/driver/api>
    $<INSTALL_INTERFACE:include/synergy/ssp/inc/driver/api>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/synergy/ssp/inc/driver/instances>
    $<INSTALL_INTERFACE:include/synergy/ssp/inc/driver/instances>
)

# Install
install(DIRECTORY synergy synergy_cfg synergy_gen DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS s7g2_sdk EXPORT s7g2_sdk-targets
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Generate exports
write_basic_package_version_file(s7g2_sdk-config-version.cmake
	VERSION ${PROJECT_VERSION}
	COMPATIBILITY SameMajorVersion)

configure_package_config_file(cmake/s7g2_sdk-config.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/s7g2_sdk-config.cmake
	INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/s7g2_sdk)

install(FILES
	${CMAKE_CURRENT_BINARY_DIR}/s7g2_sdk-config.cmake
	${CMAKE_CURRENT_BINARY_DIR}/s7g2_sdk-config-version.cmake
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/s7g2_sdk)

install(EXPORT s7g2_sdk-targets
	FILE s7g2_sdk-targets.cmake
	NAMESPACE s7g2::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/s7g2_sdk)

# Packaging
set(CPACK_PACKAGE_NAME "S7G2 SDK")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "S7G2 SDK")
set(CPACK_PACKAGE_VENDOR "Innovatrics")
set(CPACK_PACKAGE_VERSION_MAJOR ${s7g2_sdk_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${s7g2_sdk_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${s7g2_sdk_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "s7g2_sdk")
set(CPACK_STRIP_FILES FALSE)
include(CPack)

