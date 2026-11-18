cmake_minimum_required(VERSION 3.15)

project(indic VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(indic STATIC src/indic.c)
add_library(indic::indic ALIAS indic)

target_compile_features(indic PUBLIC c_std_99)
target_include_directories(indic
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(
    TARGETS indic
    EXPORT indicTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(FILES indic.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake/indicConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/indicConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/indic
)
write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/indicConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/indicConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/indicConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/indic
)
install(
    EXPORT indicTargets
    FILE indicTargets.cmake
    NAMESPACE indic::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/indic
)
