cmake_minimum_required(VERSION 3.0.0)

# test_taglib
add_executable(
    test_taglib

    # main source
    "test_taglib.cpp"
)
target_link_libraries(
    test_taglib

    # taglib
    tag tag::tag

    ${FFMPEG_LIBRARIES}
)

# test_playback
add_executable(
    test_playback

    # main source
    "test_playback.cpp"
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
    "test_player.cpp"
)
target_link_libraries(
    test_player

    # ffmpeg
    ${FFMPEG_LIBRARIES}

    # lib
    ${CMAKE_PROJECT_NAME}
)

# test/grpc

# server
add_executable(
    test_grpc_server

    # main source
    "grpc/test_grpc_server.cpp"
    "grpc/gen/helloworld.pb.cc"
    "grpc/gen/helloworld.grpc.pb.cc"
)
target_link_libraries(
    test_grpc_server

    # lib
    ${CMAKE_PROJECT_NAME}

    # grpc
    gRPC::gpr gRPC::upb gRPC::grpc gRPC::grpc++ gRPC::grpc++_reflection
)
# client
add_executable(
    test_grpc_client

    # main source
    "grpc/test_grpc_client.cpp"
    "grpc/gen/helloworld.pb.cc"
    "grpc/gen/helloworld.grpc.pb.cc"
)
target_link_libraries(
    test_grpc_client

    # lib
    ${CMAKE_PROJECT_NAME}

    # grpc
    gRPC::gpr gRPC::upb gRPC::grpc gRPC::grpc++ gRPC::grpc++_reflection
)
