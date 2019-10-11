cmake_minimum_required(VERSION 3.11)

project (uvwasi)

set(LIBUV_VERSION 1.32.0)

if (APPLE)
   set(CMAKE_MACOSX_RPATH ON)
endif (APPLE)

include(FetchContent)
## https://libuv.org
FetchContent_Declare(
        libuv
        GIT_REPOSITORY https://github.com/libuv/libuv.git
        GIT_TAG v${LIBUV_VERSION})

FetchContent_GetProperties(libuv)
if(NOT libuv_POPULATED)
    FetchContent_Populate(libuv)
    include_directories(
            "${libuv_SOURCE_DIR}/include"
    )
    add_subdirectory(
            ${libuv_SOURCE_DIR}
            ${libuv_BINARY_DIR}
            EXCLUDE_FROM_ALL)
endif()

set(SOURCES
    app.c
    src/fd_table.c
    src/uv_mapping.c
    src/uvwasi.c
)

add_executable(uvwasi ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    uv_a
)
