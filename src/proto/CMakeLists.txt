find_program(PROTOBUF_PROTOC_EXECUTABLE protoc
    PATHS /usr/bin /bin ENV PATH NO_DEFAULT_PATH)

if(NOT PROTOBUF_PROTOC_EXECUTABLE)
    message(FATAL_ERROR "Could not find 'protoc' command")
endif()

file(GLOB protos "*.proto")
foreach(proto ${protos})
    execute_process(
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE} -I${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=./ ${proto}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

file(GLOB PROTO_SRC "*.pb.cc")
file(GLOB PROTO_HDR "*.pb.h")
add_library(proto ${PROTO_SRC} ${PROTO_HDR})
