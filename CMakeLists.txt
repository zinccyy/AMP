cmake_minimum_required(VERSION 3.0.0)
project(AMP VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# libuv - async server
find_package(unofficial-libuv CONFIG REQUIRED)

# json - jsonrpc + configuration
find_package(nlohmann_json CONFIG REQUIRED)

# postgresql - database for music info
find_package(libpqxx CONFIG REQUIRED)

# taglib - audio metadata
find_package(taglib CONFIG REQUIRED)

# ffmpeg
find_package(FFMPEG REQUIRED)

# sdl2 - for testing ffmpeg
find_package(SDL2 CONFIG REQUIRED)

include_directories(
    "lib"

    # ffmpeg
    ${FFMPEG_INCLUDE_DIRS}

    # sdl2
    ${SDL2_INCLUDE_DIRS}

    # minaudio
    "libs/miniaudio"
)

link_directories(
    # ffmpeg
    ${FFMPEG_LIBRARY_DIRS}
)

set(
    LIB_SOURCES

    # main files
    "lib/amp/server.cpp"
    "lib/amp/jsonrpc.cpp"
    "lib/amp/context.cpp"
    "lib/amp/folder_hash.cpp"

    # audio
    "lib/amp/audio/player.cpp"

    # database
    "lib/amp/db/database_manager.cpp"

    # utils
    "lib/amp/utils/path.cpp"
    "lib/amp/utils/str.cpp"
)

# lib
add_library(
    ${CMAKE_PROJECT_NAME} 
    ${LIB_SOURCES}
)
target_link_libraries(
    ${CMAKE_PROJECT_NAME}

    PRIVATE

    # libuv
    unofficial::libuv::libuv 

    # json lib
    nlohmann_json nlohmann_json::nlohmann_json

    # taglib
    tag tag::tag

    # pgsql
    libpqxx::pqxx

    # ffmpeg
    ${FFMPEG_LIBRARIES}
)

# exe
add_executable(
    ${CMAKE_PROJECT_NAME}-bin
    "app/amp.cpp"
)
target_link_libraries(
    ${CMAKE_PROJECT_NAME}-bin

    PRIVATE

    # lib
    ${CMAKE_PROJECT_NAME}

    # libuv
    unofficial::libuv::libuv

    # taglib
    tag tag::tag

    # json lib
    nlohmann_json nlohmann_json::nlohmann_json

    # pgsql
    libpqxx::pqxx

    # ffmpeg
    ${FFMPEG_LIBRARIES}
)
set_target_properties(
    ${CMAKE_PROJECT_NAME}-bin
    PROPERTIES OUTPUT_NAME ${CMAKE_PROJECT_NAME}
)

# tests

# test_client
add_executable(
    test_client

    # main source
    "test/test_client.cpp"
)
target_link_libraries(
    test_client

    # lib
    ${CMAKE_PROJECT_NAME}

    # libuv
    unofficial::libuv::libuv 

    # taglib
    tag tag::tag

    # json lib
    nlohmann_json nlohmann_json::nlohmann_json
)


# test_taglib
add_executable(
    test_taglib

    # main source
    "test/test_taglib.cpp"
)
target_link_libraries(
    test_taglib

    # taglib
    tag tag::tag

    ${FFMPEG_LIBRARIES}
)

# test_ffmpeg
add_executable(
    test_ffmpeg

    # main source
    "test/test_ffmpeg.cpp"
)
target_link_libraries(
    test_ffmpeg

    # ffmpeg
    ${FFMPEG_LIBRARIES}

    # sdl2
    SDL2::SDL2
)

# test_playback
add_executable(
    test_playback

    # main source
    "test/test_playback.cpp"
)
target_link_libraries(
    test_playback

    # ffmpeg
    ${FFMPEG_LIBRARIES}
)

# test_player
add_executable(
    test_player

    # main source
    "test/test_player.cpp"
)
target_link_libraries(
    test_player

    # ffmpeg
    ${FFMPEG_LIBRARIES}

    # lib
    ${CMAKE_PROJECT_NAME}
)
