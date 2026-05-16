cmake_minimum_required(VERSION 3.20)
project(neml2-hit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Flex + Bison ───────────────────────────────────────────────────────────────
# Debug builds regenerate the parser/lexer from source (flex + bison required).
# All other build types (Release, RelWithDebInfo, MinSizeRel, …) use the
# pre-generated files committed to generated/ and need no flex or bison.

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE)
  find_package(FLEX 2.6 REQUIRED)
  find_package(BISON 3.7 REQUIRED)

  FLEX_TARGET(
    HITLexer
    src/Lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Lexer.h
  )

  BISON_TARGET(
    HITParser
    src/Parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Parser.h
  )

  ADD_FLEX_BISON_DEPENDENCY(HITLexer HITParser)

  set(_parser_sources
    ${FLEX_HITLexer_OUTPUTS}
    ${BISON_HITParser_OUTPUTS}
  )
  set(_parser_include_dir ${CMAKE_CURRENT_BINARY_DIR})

  # Helper target: copy freshly-generated files back to generated/ for committing.
  add_custom_target(update_generated
    COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/Lexer.h
      ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp
      ${CMAKE_CURRENT_BINARY_DIR}/Parser.h
      ${CMAKE_CURRENT_BINARY_DIR}/location.hh
      ${CMAKE_CURRENT_SOURCE_DIR}/generated/
    DEPENDS
      ${FLEX_HITLexer_OUTPUTS}
      ${BISON_HITParser_OUTPUTS}
    COMMENT "Copying generated parser/lexer files to generated/"
  )
else()
  set(_parser_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Lexer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Lexer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/Parser.h
    ${CMAKE_CURRENT_SOURCE_DIR}/generated/location.hh
  )
  set(_parser_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/generated)
endif()

# ── library ───────────────────────────────────────────────────────────────────

add_library(nmhit
  ${_parser_sources}
  src/Node.cpp
  src/BraceExpr.cpp
)

target_include_directories(nmhit
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src
    ${_parser_include_dir}
)

target_compile_options(nmhit PRIVATE -Wall -Wextra -Wno-unused-parameter)

# ── install ───────────────────────────────────────────────────────────────────
# C++ library install rules are skipped when building the Python wheel so that
# headers, CMake config files, and pkg-config files don't end up inside the wheel.

if(NOT NMHIT_BUILD_PYTHON)
  include(CMakePackageConfigHelpers)

  install(TARGETS nmhit
    EXPORT nmhitTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
  )

  install(DIRECTORY include/
    DESTINATION include
  )

  configure_package_config_file(
    cmake/nmhitConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/nmhitConfig.cmake
    INSTALL_DESTINATION lib/cmake/nmhit
  )

  write_basic_package_version_file(
    ${CMAKE_CURRENT_BINARY_DIR}/nmhitConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
  )

  install(EXPORT nmhitTargets
    FILE nmhitTargets.cmake
    NAMESPACE nmhit::
    DESTINATION lib/cmake/nmhit
  )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/nmhitConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/nmhitConfigVersion.cmake
    DESTINATION lib/cmake/nmhit
  )

  configure_file(
    cmake/nmhit.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/nmhit.pc
    @ONLY
  )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/nmhit.pc
    DESTINATION lib/pkgconfig
  )
endif()

# ── tests ──────────────────────────────────────────────────────────────────────

option(NMHIT_BUILD_TESTS "Build nmhit unit tests" ON)
if(NMHIT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# ── Python bindings (nanobind) ─────────────────────────────────────────────────
# Enabled by scikit-build-core via -DNMHIT_BUILD_PYTHON=ON.
# Release builds use the pre-generated parser/lexer in generated/ so flex and
# bison are not required on the wheel-build host.

option(NMHIT_BUILD_PYTHON "Build Python bindings via nanobind" OFF)
if(NMHIT_BUILD_PYTHON)
  find_package(Python 3.9 REQUIRED COMPONENTS Interpreter Development.Module)

  # Locate nanobind's CMake integration through the Python interpreter.
  # scikit-build-core installs nanobind into the build venv so it is findable
  # via `python -m nanobind --cmake_dir`.
  execute_process(
    COMMAND "${Python_EXECUTABLE}" -m nanobind --cmake_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    OUTPUT_VARIABLE _NB_CMAKE_DIR
    RESULT_VARIABLE _NB_RESULT
  )
  if(NOT _NB_RESULT EQUAL 0)
    message(FATAL_ERROR "Could not locate nanobind CMake directory. "
                        "Install it with: pip install nanobind")
  endif()
  list(APPEND CMAKE_PREFIX_PATH "${_NB_CMAKE_DIR}")
  find_package(nanobind CONFIG REQUIRED)

  nanobind_add_module(
    _nmhit
    STABLE_ABI  # Py_LIMITED_API=0x03090000: one .so covers Python 3.9+
    NB_STATIC   # embed nanobind runtime; wheel is self-contained
    python/src/_nmhit.cpp
  )

  target_link_libraries(_nmhit PRIVATE nmhit)

  # Install into the nmhit/ subdirectory inside the wheel.
  # scikit-build-core picks this up via wheel.packages = ["python/nmhit"].
  install(TARGETS _nmhit DESTINATION nmhit)
endif()
