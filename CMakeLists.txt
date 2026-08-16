cmake_minimum_required(VERSION 3.20)
project(OpenEphemeris VERSION 0.1.0 LANGUAGES C)

option(OE_BUILD_SHARED "Build the shared library" ON)
option(OE_BUILD_TESTS "Build tests" ON)
option(OE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" ON)

set(OE_SOURCES
    src/oe_api.c
    src/oe_time.c
    src/oe_spk.c
    src/oe_astro.c
    src/oe_lunar.c
    src/oe_houses.c)

file(GLOB ERFA_SOURCES CONFIGURE_DEPENDS third_party/erfa/src/*.c)
list(FILTER ERFA_SOURCES EXCLUDE REGEX "/t_erfa_c(_extra)?\\.c$")
list(FILTER ERFA_SOURCES EXCLUDE REGEX "/erfaversion\\.c$")
add_library(oe_erfa OBJECT ${ERFA_SOURCES})
target_include_directories(oe_erfa PUBLIC third_party/erfa/src)
set_target_properties(oe_erfa PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(openephemeris_static STATIC ${OE_SOURCES})
add_library(OpenEphemeris::static ALIAS openephemeris_static)
set_target_properties(openephemeris_static PROPERTIES OUTPUT_NAME openephemeris)
target_compile_definitions(openephemeris_static PUBLIC OE_STATIC)
target_compile_features(openephemeris_static PUBLIC c_std_11)
target_include_directories(openephemeris_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src third_party/erfa/src)
target_sources(openephemeris_static PRIVATE $<TARGET_OBJECTS:oe_erfa>)

if(OE_BUILD_SHARED)
  add_library(openephemeris_shared SHARED ${OE_SOURCES})
  add_library(OpenEphemeris::shared ALIAS openephemeris_shared)
  set_target_properties(openephemeris_shared PROPERTIES OUTPUT_NAME openephemeris
      C_VISIBILITY_PRESET hidden VISIBILITY_INLINES_HIDDEN YES)
  target_compile_features(openephemeris_shared PUBLIC c_std_11)
  target_compile_definitions(openephemeris_shared PRIVATE OE_BUILDING_DLL)
  target_include_directories(openephemeris_shared PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include>
      PRIVATE src third_party/erfa/src)
  target_sources(openephemeris_shared PRIVATE $<TARGET_OBJECTS:oe_erfa>)
endif()

foreach(target openephemeris_static openephemeris_shared)
  if(TARGET ${target})
    if(MSVC)
      target_compile_options(${target} PRIVATE /W4 $<$<BOOL:${OE_WARNINGS_AS_ERRORS}>:/WX>)
    else()
      target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic
          $<$<BOOL:${OE_WARNINGS_AS_ERRORS}>:-Werror>)
      target_link_libraries(${target} PRIVATE m)
    endif()
  endif()
endforeach()

include(GNUInstallDirs)
install(TARGETS openephemeris_static
    EXPORT OpenEphemerisTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
if(OE_BUILD_SHARED)
  install(TARGETS openephemeris_shared
      EXPORT OpenEphemerisTargets
      LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
      RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(OE_BUILD_TESTS)
  enable_testing()
  add_executable(oe_tests tests/test_main.c)
  target_link_libraries(oe_tests PRIVATE OpenEphemeris::static)
  add_test(NAME oe_tests COMMAND oe_tests)
endif()

add_executable(oe-example examples/positions.c)
target_link_libraries(oe-example PRIVATE OpenEphemeris::static)
