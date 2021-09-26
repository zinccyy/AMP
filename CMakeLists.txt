cmake_minimum_required(VERSION 3.0.0)
project(AMP VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
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

# grpc
find_package(gRPC CONFIG REQUIRED)

include_directories(
    "lib"

    "grpc/protobuf/cpp"

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
    "lib/amp/context.cpp"
    "lib/amp/folder_hash.cpp"

    # rpc
    "lib/amp/rpc/server.cpp"

    # audio
    "lib/amp/audio/player.cpp"

    # database
    "lib/amp/db/database_manager.cpp"

    # rpc
    "lib/amp/rpc/server.cpp"

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

    # grpc
    gRPC::gpr gRPC::upb gRPC::grpc gRPC::grpc++ gRPC::grpc++_reflection
)

# exe
add_executable(
    ${CMAKE_PROJECT_NAME}-bin
    "app/amp.cpp"

    # services

    # common
    "grpc/protobuf/cpp/common.grpc.pb.cc"
    "grpc/protobuf/cpp/common.pb.cc"

    # database_service
    "lib/amp/rpc/services/database_service/database_service.cpp"
    "grpc/protobuf/cpp/database.grpc.pb.cc"
    "grpc/protobuf/cpp/database.pb.cc"

    # player_service
    "lib/amp/rpc/services/player_service/player_service.cpp"
    "grpc/protobuf/cpp/player.grpc.pb.cc"
    "grpc/protobuf/cpp/player.pb.cc"

    # system service
    "lib/amp/rpc/services/sys_service/sys_service.cpp"
    "grpc/protobuf/cpp/sys.grpc.pb.cc"
    "grpc/protobuf/cpp/sys.pb.cc"
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
add_subdirectory(
    "test"
)