cmake_minimum_required(VERSION 3.14)

project(ngl-pokepasteTests LANGUAGES CXX)

include(../cmake/project-is-top-level.cmake)
include(../cmake/folders.cmake)

# ---- Dependencies ----

if(PROJECT_IS_TOP_LEVEL)
  find_package(ngl-pokepaste REQUIRED)
  enable_testing()
endif()

# ---- Tests ----

add_executable(ngl-pokepaste_test source/ngl-pokepaste_test.cpp)
target_link_libraries(ngl-pokepaste_test PRIVATE ngl-pokepaste::ngl-pokepaste)
target_compile_features(ngl-pokepaste_test PRIVATE cxx_std_20)

add_custom_command(
  TARGET ngl-pokepaste_test POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/resources" $<TARGET_FILE_DIR:ngl-pokepaste_test>/resources
)

add_test(NAME ngl-pokepaste_test COMMAND ngl-pokepaste_test)

# ---- End-of-file commands ----

add_folders(Test)
