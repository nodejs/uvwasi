cmake_minimum_required(VERSION 3.11)
project (
    uvwasi
    DESCRIPTION "WASI syscall API built atop libuv"
    VERSION 0.0.21
    LANGUAGES C
)

# Point CMake at any custom modules we may ship
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# This can be a commit hash or tag
set(LIBUV_VERSION v1.44.2)

include(CMakeDependentOption)
cmake_dependent_option(BUILD_TESTING
  "Build the unit tests when uvwasi is the root project" ON
  "CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR" OFF)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
  list(APPEND uvwasi_cflags -fvisibility=hidden --std=gnu89)
  list(APPEND uvwasi_cflags -Wall -Wsign-compare -Wextra -Wstrict-prototypes)
  list(APPEND uvwasi_cflags -Wno-unused-parameter)
endif()

if(APPLE)
   set(CMAKE_MACOSX_RPATH ON)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  list(APPEND uvwasi_defines _GNU_SOURCE _POSIX_C_SOURCE=200112)
endif()

find_package(LIBUV QUIET)
if(LIBUV_FOUND)
  include_directories(${LIBUV_INCLUDE_DIR})
else()
  include(FetchContent)
  ## https://libuv.org
  FetchContent_Declare(
          libuv
          GIT_REPOSITORY https://github.com/libuv/libuv.git
          GIT_TAG ${LIBUV_VERSION})

  FetchContent_GetProperties(libuv)
  if(NOT libuv_POPULATED)
      FetchContent_Populate(libuv)
      include_directories("${libuv_SOURCE_DIR}/include")
      add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()
  set(LIBUV_INCLUDE_DIR ${libuv_SOURCE_DIR}/include)
  set(LIBUV_LIBRARIES uv_a)
endif()

## uvwasi source code files.
set(uvwasi_sources
    src/clocks.c
    src/fd_table.c
    src/path_resolver.c
    src/poll_oneoff.c
    src/sync_helpers.c
    src/uv_mapping.c
    src/uvwasi.c
    src/wasi_rights.c
    src/wasi_serdes.c
)

option(UVWASI_DEBUG_LOG "Enable debug logging" OFF)
if(UVWASI_DEBUG_LOG)
    list(APPEND uvwasi_cflags -DUVWASI_DEBUG_LOG)
endif()

# Code Coverage Configuration
add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_C_COMPILER_ID MATCHES "AppleClang|GNU|Clang")
    # Add required flags (GCC & LLVM/Clang)
    target_compile_options(coverage_config INTERFACE
        -O0        # no optimization
        -g         # generate debug info
        --coverage # sets all required flags
    )
    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
        target_link_options(coverage_config INTERFACE --coverage)
    else()
        target_link_libraries(coverage_config INTERFACE --coverage)
    endif()
endif()

# ASAN Support
option(ASAN "Enable code asan" OFF)
if(ASAN AND CMAKE_C_COMPILER_ID MATCHES "AppleClang|GNU|Clang")
    set (CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
    set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

## Static library target.
add_library(uvwasi_a STATIC ${uvwasi_sources})
set_target_properties(uvwasi_a PROPERTIES OUTPUT_NAME "uvwasi")
target_compile_definitions(uvwasi_a PRIVATE ${uvwasi_defines})
target_compile_options(uvwasi_a PRIVATE ${uvwasi_cflags})
target_include_directories(uvwasi_a PRIVATE ${PROJECT_SOURCE_DIR}/include)
if(CODE_COVERAGE)
    target_link_libraries(uvwasi_a PUBLIC ${LIBUV_LIBRARIES} coverage_config)
else()
    target_link_libraries(uvwasi_a PRIVATE ${LIBUV_LIBRARIES})
endif()

## Shared library target.
add_library(uvwasi SHARED ${uvwasi_sources})
target_compile_definitions(uvwasi PRIVATE ${uvwasi_defines})
target_compile_options(uvwasi PRIVATE ${uvwasi_cflags})
target_include_directories(uvwasi PRIVATE ${PROJECT_SOURCE_DIR}/include)
if(CODE_COVERAGE)
    target_link_libraries(uvwasi PUBLIC ${LIBUV_LIBRARIES} coverage_config)
else()
    target_link_libraries(uvwasi PRIVATE ${LIBUV_LIBRARIES})
endif()


## Test targets.
if(BUILD_TESTING)
    enable_testing()
    file(GLOB test_files "test/test-*.c")
    foreach(file ${test_files})
        get_filename_component(test_name ${file} NAME_WE)
        add_executable(${test_name} ${file})
        add_test(NAME ${test_name}
                    COMMAND ${test_name})
        target_include_directories(${test_name}
                                    PRIVATE
                                    ${PROJECT_SOURCE_DIR}/include)
        target_link_libraries(${test_name} PRIVATE ${LIBUV_LIBRARIES} uvwasi_a)
        list(APPEND test_list ${test_name})
    endforeach()

    add_custom_target(check
        COMMAND ctest -VV -C Debug -R test-
        DEPENDS ${test_list}
    )
endif()

option(INSTALL_UVWASI "Enable installation of uvwasi. (Projects embedding uvwasi may want to turn this OFF.)" ON)
if(INSTALL_UVWASI AND NOT CODE_COVERAGE)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    set(target_export_name ${PROJECT_NAME}Targets CACHE INTERNAL "")
    set(cmake_files_install_dir ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})

    set(pkg_config ${PROJECT_BINARY_DIR}/uvwasi.pc)
    set(version_file ${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake)
    set(config_file ${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake)

    configure_file(${PROJECT_SOURCE_DIR}/cmake/uvwasi.pc.in ${pkg_config} @ONLY)
    write_basic_package_version_file(${version_file} VERSION ${PROJECT_VERSION} COMPATIBILITY AnyNewerVersion)
    configure_package_config_file(${PROJECT_SOURCE_DIR}/cmake/Config.cmake.in ${config_file} INSTALL_DESTINATION ${cmake_files_install_dir})

    install(
        TARGETS uvwasi_a uvwasi
        EXPORT ${target_export_name}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
    install(
        DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING PATTERN "*.h"
    )
    install(
        FILES ${pkg_config}
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
    )
    install(
        EXPORT ${target_export_name}
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${cmake_files_install_dir}
    )
    install(
        FILES ${version_file} ${config_file}
        DESTINATION ${cmake_files_install_dir}
    )
endif()

message(STATUS "summary of uvwasi build options:

    Install prefix:  ${CMAKE_INSTALL_PREFIX}
    Target system:   ${CMAKE_SYSTEM_NAME}
    Compiler:
      C compiler:    ${CMAKE_C_COMPILER}
      CFLAGS:        ${CMAKE_C_FLAGS_${_build_type}} ${CMAKE_C_FLAGS}

    LibUV libraries: ${LIBUV_LIBRARIES}
    LibUV includes:  ${LIBUV_INCLUDE_DIR}
    Debug logging:   ${UVWASI_DEBUG_LOG}
    Code coverage:   ${CODE_COVERAGE}
    ASAN:            ${ASAN}
    Build tests:     ${UVWASI_BUILD_TESTS}
")
