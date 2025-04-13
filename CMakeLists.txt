cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
    ngl-pokepaste
    VERSION 0.1.0
    DESCRIPTION "Short description"
    HOMEPAGE_URL "https://example.com/"
    LANGUAGES NONE
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(ngl-pokepaste_ngl-pokepaste INTERFACE)
add_library(ngl-pokepaste::ngl-pokepaste ALIAS ngl-pokepaste_ngl-pokepaste)

set_property(
    TARGET ngl-pokepaste_ngl-pokepaste PROPERTY
    EXPORT_NAME ngl-pokepaste
)

target_include_directories(
    ngl-pokepaste_ngl-pokepaste ${warning_guard}
    INTERFACE
    "\$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(ngl-pokepaste_ngl-pokepaste INTERFACE cxx_std_20)

# ---- Install rules ----

if(NOT CMAKE_SKIP_INSTALL_RULES)
  include(cmake/install-rules.cmake)
endif()

# ---- Developer mode ----

if(NOT ngl-pokepaste_DEVELOPER_MODE)
  return()
elseif(NOT PROJECT_IS_TOP_LEVEL)
  message(
      AUTHOR_WARNING
      "Developer mode is intended for developers of ngl-pokepaste"
  )
endif()

include(cmake/dev-mode.cmake)
