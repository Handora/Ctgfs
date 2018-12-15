file(GLOB protos "*.proto")
foreach(proto ${protos})
    execute_process(
        COMMAND ${PROJECT_SOURCE_DIR}/third_party/protobuf/build/bin/protoc -I${CMAKE_CURRENT_SOURCE_DIR} --cpp_out=./ ${proto}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endforeach()

file(GLOB PROTO_SRC "*.pb.cc")
file(GLOB PROTO_HDR "*.pb.h")
add_library(proto ${PROTO_SRC} ${PROTO_HDR})
