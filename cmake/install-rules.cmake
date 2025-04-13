if(PROJECT_IS_TOP_LEVEL)
  set(
      CMAKE_INSTALL_INCLUDEDIR "include/ngl-pokepaste-${PROJECT_VERSION}"
      CACHE STRING ""
  )
  set_property(CACHE CMAKE_INSTALL_INCLUDEDIR PROPERTY TYPE PATH)
endif()

# Project is configured with no languages, so tell GNUInstallDirs the lib dir
set(CMAKE_INSTALL_LIBDIR lib CACHE PATH "")

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

# find_package(<package>) call for consumers to find this project
set(package ngl-pokepaste)

install(
    DIRECTORY include/
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    COMPONENT ngl-pokepaste_Development
)

install(
    TARGETS ngl-pokepaste_ngl-pokepaste
    EXPORT ngl-pokepasteTargets
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

write_basic_package_version_file(
    "${package}ConfigVersion.cmake"
    COMPATIBILITY SameMajorVersion
    ARCH_INDEPENDENT
)

# Allow package maintainers to freely override the path for the configs
set(
    ngl-pokepaste_INSTALL_CMAKEDIR "${CMAKE_INSTALL_DATADIR}/${package}"
    CACHE STRING "CMake package config location relative to the install prefix"
)
set_property(CACHE ngl-pokepaste_INSTALL_CMAKEDIR PROPERTY TYPE PATH)
mark_as_advanced(ngl-pokepaste_INSTALL_CMAKEDIR)

install(
    FILES cmake/install-config.cmake
    DESTINATION "${ngl-pokepaste_INSTALL_CMAKEDIR}"
    RENAME "${package}Config.cmake"
    COMPONENT ngl-pokepaste_Development
)

install(
    FILES "${PROJECT_BINARY_DIR}/${package}ConfigVersion.cmake"
    DESTINATION "${ngl-pokepaste_INSTALL_CMAKEDIR}"
    COMPONENT ngl-pokepaste_Development
)

install(
    EXPORT ngl-pokepasteTargets
    NAMESPACE ngl-pokepaste::
    DESTINATION "${ngl-pokepaste_INSTALL_CMAKEDIR}"
    COMPONENT ngl-pokepaste_Development
)

if(PROJECT_IS_TOP_LEVEL)
  include(CPack)
endif()
