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

# ── tests ──────────────────────────────────────────────────────────────────────

option(NMHIT_BUILD_TESTS "Build nmhit unit tests" ON)
if(NMHIT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
