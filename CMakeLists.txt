cmake_minimum_required(VERSION 3.20)
# Keep this version in sync with [project] version in pyproject.toml.
project(neml2-hit VERSION 0.3.6 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# MSVC reports __cplusplus == 199711L by default even under /std:c++17, which
# makes standards-conditional code take its pre-C++11 branch -- notably the
# Bison-generated parser, whose value type is a move-only std::unique_ptr<Node>
# it would otherwise try to copy. Report the true value for every target here
# (the library, the Python module, and the tests) rather than per-target.
if(MSVC)
  add_compile_options(/Zc:__cplusplus)
endif()

# ── Flex + Bison ───────────────────────────────────────────────────────────────
# Single-config Debug (or no-build-type) builds regenerate the parser/lexer from
# source (flex + bison required). Every other case uses the pre-generated files
# committed to generated/ and needs no flex or bison:
#   - single-config Release/RelWithDebInfo/MinSizeRel, and
#   - ALL multi-config generators (Visual Studio, Ninja Multi-Config), where the
#     per-config build type is unknown at configure time so CMAKE_BUILD_TYPE is
#     empty by design. This is the wheel-build path on Windows (scikit-build-core
#     drives the Visual Studio generator), which must NOT require flex/bison.
get_property(_nmhit_multi_config GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT _nmhit_multi_config AND (CMAKE_BUILD_TYPE STREQUAL "Debug" OR NOT CMAKE_BUILD_TYPE))
  find_package(FLEX 2.6 REQUIRED)
  find_package(BISON 3.7 REQUIRED)

  FLEX_TARGET(
    HITLexer
    src/Lexer.l
    ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Lexer.h
    COMPILE_FLAGS "--noline"
  )

  BISON_TARGET(
    HITParser
    src/Parser.y
    ${CMAKE_CURRENT_BINARY_DIR}/Parser.cpp
    DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/Parser.h
    COMPILE_FLAGS "--no-lines"
  )

  ADD_FLEX_BISON_DEPENDENCY(HITLexer HITParser)

  set(_parser_sources
    ${FLEX_HITLexer_OUTPUTS}
    ${BISON_HITParser_OUTPUTS}
  )
  set(_parser_include_dir ${CMAKE_CURRENT_BINARY_DIR})

  # Helper target: copy freshly-generated files back to generated/ for committing.
  # The sed step removes any residual #line directives (flex emits one before
  # %top{} even with --noline) so no local filesystem paths leak into the repo.
  set(_gen_dest ${CMAKE_CURRENT_SOURCE_DIR}/generated)
  add_custom_target(update_generated
    COMMAND ${CMAKE_COMMAND} -E copy
      Lexer.cpp Lexer.h Parser.cpp Parser.h location.hh ${_gen_dest}/
    COMMAND sed -i "/^#line /d"
      ${_gen_dest}/Lexer.cpp
      ${_gen_dest}/Lexer.h
      ${_gen_dest}/Parser.cpp
      ${_gen_dest}/Parser.h
      ${_gen_dest}/location.hh
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS
      ${FLEX_HITLexer_OUTPUTS}
      ${BISON_HITParser_OUTPUTS}
    COMMENT "Stripping #line directives and copying generated parser/lexer files to generated/"
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
  # Weak __cxa_call_terminate fallback so the static archive links into consumers
  # built with an older toolchain than the (GCC >= 13) wheel build. See the file.
  src/abi_compat.cpp
)

target_include_directories(nmhit
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
  PRIVATE
    src
    ${_parser_include_dir}
)

# Warning flags are compiler-specific: GCC/Clang take the -W family, MSVC takes
# /W. Passing the GCC/Clang flags to cl.exe (as an unguarded list would) makes
# MSVC choke -- the original blocker reported in neml2 issue #414.
target_compile_options(nmhit PRIVATE
  $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall;-Wextra;-Wno-unused-parameter>
  $<$<CXX_COMPILER_ID:MSVC>:/W3>
)

# MSVC-specific compile settings for the parser/lexer:
#  - YY_NO_UNISTD_H: the flex-generated Lexer.h unconditionally pulls in
#    <unistd.h> (a POSIX header absent on MSVC) unless this is defined. The
#    Lexer.l %top{} defines it for Lexer.cpp, but not for the DEFINES_FILE
#    header, so define it target-wide to cover every translation unit.
#  - _CRT_SECURE_NO_WARNINGS: silences C4996 on std::getenv (BraceExpr.cpp).
target_compile_definitions(nmhit PRIVATE
  $<$<PLATFORM_ID:Windows>:YY_NO_UNISTD_H>
  $<$<CXX_COMPILER_ID:MSVC>:_CRT_SECURE_NO_WARNINGS>
)

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
  # Per-version wheels: only Development.Module is needed here.
  # When Python 3.12 becomes the project minimum, switch to a single abi3 wheel:
  #   - set wheel.py-api = "cp312" in pyproject.toml
  #   - add Development.SABIModule to this find_package call (requires CMake >= 3.26)
  #   - add STABLE_ABI back to nanobind_add_module below
  # nanobind's STABLE_ABI silently no-ops below Python 3.12 (Py_LIMITED_API=0x030C0000).
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
    NB_STATIC   # embed nanobind runtime; wheel is self-contained
    python/src/_nmhit.cpp
  )

  target_link_libraries(_nmhit PRIVATE nmhit)

  # Install into the nmhit/ subdirectory inside the wheel.
  # scikit-build-core picks this up via wheel.packages = ["python/nmhit"].
  install(TARGETS _nmhit DESTINATION nmhit)

  # Also ship the C++ dev files (the static libnmhit + public headers) inside the
  # wheel's nmhit/ package, so downstream C++ consumers (e.g. NEML2's
  # libneml2_aoti) can discover and link nmhit straight from site-packages via a
  # Find module -- the same way libtorch is found in torch/{include,lib}. nmhit
  # is built PIC (CMAKE_POSITION_INDEPENDENT_CODE=ON) so the static archive links
  # cleanly into a shared library.
  install(TARGETS nmhit ARCHIVE DESTINATION nmhit/lib)
  install(DIRECTORY include/ DESTINATION nmhit/include)
endif()
