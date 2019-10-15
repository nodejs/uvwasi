cmake_minimum_required(VERSION 3.11)
project (uvwasi LANGUAGES C)

set(LIBUV_VERSION 1.32.0)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

if(CMAKE_C_COMPILER_ID MATCHES "AppleClang|Clang|GNU")
  list(APPEND uvwasi_cflags -fvisibility=hidden --std=gnu89)
  list(APPEND uvwasi_cflags -Wall -Wsign-compare -Wextra -Wstrict-prototypes)
  list(APPEND uvwasi_cflags -Wno-unused-parameter)
endif()

if(APPLE)
   set(CMAKE_MACOSX_RPATH ON)
endif()

include(FetchContent)
## https://libuv.org
FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG v${LIBUV_VERSION})

FetchContent_GetProperties(libuv)
if(NOT libuv_POPULATED)
    FetchContent_Populate(libuv)
    include_directories("${libuv_SOURCE_DIR}/include")
    add_subdirectory(${libuv_SOURCE_DIR} ${libuv_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

## uvwasi source code files.
set(uvwasi_sources
    src/fd_table.c
    src/uv_mapping.c
    src/uvwasi.c
)


## Static library target.
add_library(uvwasi_a STATIC ${uvwasi_sources})
target_compile_options(uvwasi_a PRIVATE ${uvwasi_cflags})
target_include_directories(uvwasi_a PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(uvwasi_a PRIVATE uv_a)


## Demo app target.
set(app_sources app.c)
add_executable(app ${app_sources})
target_compile_options(app PRIVATE ${uvwasi_cflags})
target_include_directories(app PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(app PRIVATE uv_a uvwasi_a)


## Test targets.
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
    target_link_libraries(${test_name} PRIVATE uv_a uvwasi_a)
    list(APPEND test_list ${test_name})
endforeach()

add_custom_target(check
    COMMAND ctest -R test-
    DEPENDS ${test_list}
)

message(STATUS "summary of build options:

    Install prefix:  ${CMAKE_INSTALL_PREFIX}
    Target system:   ${CMAKE_SYSTEM_NAME}
    Compiler:
      C compiler:    ${CMAKE_C_COMPILER}
      CFLAGS:        ${CMAKE_C_FLAGS_${_build_type}} ${CMAKE_C_FLAGS}
    Libuv version:   ${LIBUV_VERSION}
    Libuv lib:       ${libuv_BINARY_DIR}
")
