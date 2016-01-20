CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

# UNTAR 3RD PARTY LIB
ADD_CUSTOM_TARGET(3rd_lib
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/untarlib.sh
    ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party 3rd_lib.tar.gz
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/ 
    COMMENT "Prebuild: unarchive 3rd party lib")

SET(THIRD_PARTY_SRC ../3rd_party/src/)
SET(THIRD_PARTY_LIB ${CMAKE_CURRENT_SOURCE_DIR}/../3rd_party/lib/)

#########################################################################
#########################################################################

PROJECT(msh)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS}  -O0 -Wall -Wno-unused-local-typedefs -g -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS}  -O2 -g -Wall -Wno-unused-local-typedefs") 

# CMAKE build information OUTPUT
IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(STATUS "Debug Mode")
    MESSAGE(STATUS "Flags:" ${CMAKE_CXX_FLAGS_DEBUG})
ELSE (CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(STATUS "Release Mode")
    MESSAGE(STATUS "Flags:" ${CMAKE_CXX_FLAGS_RELEASE})
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

add_definitions(-DSIGNED_RIGHT_SHIFT_IS)
add_definitions(-DARITHMETIC_RIGHT_SHIFT)
add_definitions(-DHAVE_NETINET_IN_H)
add_definitions(-DHAVE_INTTYPES_H)
add_definitions(-DHAVE_NETDB_H)

INCLUDE_DIRECTORIES(${THIRD_PARTY_SRC})
INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(./framework)

#
#set extern libraries
#
SET(IM_LIBRARIES
libpthread.so
libdl.so
librt.so

${THIRD_PARTY_LIB}/libthriftnb.a
${THIRD_PARTY_LIB}/libthrift.a
${THIRD_PARTY_LIB}/libevent.a
${THIRD_PARTY_LIB}/libglog.a
${THIRD_PARTY_LIB}/libboost_system.a
${THIRD_PARTY_LIB}/libboost_regex.a
${THIRD_PARTY_LIB}/libprotobuf.a
${THIRD_PARTY_LIB}/libtcmalloc.a
${THIRD_PARTY_LIB}/libzookeeper_mt.a
${THIRD_PARTY_LIB}/libjson_linux-gcc-4.1.2_libmt.a
${THIRD_PARTY_LIB}/libhiredis.a
)

SET(IM_PROJECT_SRC
    framework/io/io_accept.cpp
    framework/io/io_app.cpp
    framework/io/io_app_accept_service.cpp
    framework/io/io_app_cluster.cpp
    framework/io/io_app_connection.cpp
    framework/io/io_connection.cpp
    framework/io/io_manager.cpp
    framework/io/io_sampling.cpp

    framework/ms/ms_message.cpp
    framework/ms/ms_message_block_queue.cpp
    framework/ms/ms_message_priority_queue.cpp
    framework/ms/ms_message_unsafe_queue.cpp
    framework/ms/ms_message_collector.cpp
    framework/ms/ms_message_queue.cpp
    framework/ms/ms_message_router_local.cpp
    framework/ms/ms_worker.cpp

    framework/lib/block_queue.cpp
    framework/lib/posix_thread_factory.cpp
    framework/lib/queue.cpp
    framework/lib/util.cpp
    framework/lib/redis_operator.cpp
    framework/lib/simple_queue.cpp
    framework/lib/priority_queue.cpp
    framework/lib/ini_file_reader.cpp
    framework/lib/string_algorithms.cpp

    
    message/im_message_dispatcher.cpp
    message/im_message_processor.cpp
    message/im_pending_message_collector_core.cpp
    message/im_user_channel_register_table_local.cpp
    message/im_user_message_router.cpp
    message/im_message_parser.cpp
    message/im_app_worker_cluster.cpp
    message/im_app_message_core.cpp
    message/im_auth_worker_cluster.cpp
    message/im_auth_message_core.cpp
    message/im_message_auth_processor.cpp
    message/route_info_publisher.cpp
)

AUX_SOURCE_DIRECTORY(./  IM_PROJECT_SRC)
AUX_SOURCE_DIRECTORY(./proto IM_PROJECT_SRC)
AUX_SOURCE_DIRECTORY(./thrift/broker IM_PROJECT_SRC)
AUX_SOURCE_DIRECTORY(./thrift/gen IM_PROJECT_SRC)
AUX_SOURCE_DIRECTORY(./thrift/pilot IM_PROJECT_SRC)

ADD_EXECUTABLE(msh ${IM_PROJECT_SRC})
ADD_DEPENDENCIES(msh 3rd_lib)

TARGET_LINK_LIBRARIES(msh ${IM_LIBRARIES})
SET(EXECUTABLE_OUTPUT_PATH ./bin)

#########################################################################
#########################################################################

PROJECT(touch)

INCLUDE_DIRECTORIES(${THIRD_PARTY_SRC})
INCLUDE_DIRECTORIES(.)

SET(TOUCH_LIBRARIES
libpthread.so
libdl.so
librt.so

${THIRD_PARTY_LIB}/libevent.a
${THIRD_PARTY_LIB}/libglog.a
${THIRD_PARTY_LIB}/libprotobuf.a
${THIRD_PARTY_LIB}/libtcmalloc.a
)

AUX_SOURCE_DIRECTORY(./proto TOUCH_PROJECT_SRCS)
ADD_EXECUTABLE(touch ./client/touch.cpp ${TOUCH_PROJECT_SRCS})
ADD_DEPENDENCIES(touch 3rd_lib)

TARGET_LINK_LIBRARIES(touch ${TOUCH_LIBRARIES})
SET(EXECUTABLE_OUTPUT_PATH ./bin)

#########################################################################
#########################################################################

PROJECT(faketouch)

INCLUDE_DIRECTORIES(${THIRD_PARTY_SRC})
INCLUDE_DIRECTORIES(.)

SET(FAKE_TOUCH_LIBRARIES
libpthread.so
libdl.so
librt.so

${THIRD_PARTY_LIB}/libevent.a
${THIRD_PARTY_LIB}/libglog.a
${THIRD_PARTY_LIB}/libprotobuf.a
${THIRD_PARTY_LIB}/libtcmalloc.a
)

AUX_SOURCE_DIRECTORY(./proto FAKE_TOUCH_PROJECT_SRCS)
ADD_EXECUTABLE(faketouch ./client/fakeTouch.cpp ${FAKE_TOUCH_PROJECT_SRCS})
ADD_DEPENDENCIES(faketouch 3rd_lib)

TARGET_LINK_LIBRARIES(faketouch ${FAKE_TOUCH_LIBRARIES})
SET(EXECUTABLE_OUTPUT_PATH ./bin)

#########################################################################
#########################################################################

PROJECT(client)

INCLUDE_DIRECTORIES(${THIRD_PARTY_SRC})
INCLUDE_DIRECTORIES(.)

SET(CLIENT_LIBRARIES
libpthread.so
libdl.so
librt.so

${THIRD_PARTY_LIB}/libevent.a
${THIRD_PARTY_LIB}/libglog.a
${THIRD_PARTY_LIB}/libprotobuf.a
${THIRD_PARTY_LIB}/libtcmalloc.a
${THIRD_PARTY_LIB}/libhiredis.a
${THIRD_PARTY_LIB}/libboost_regex.a
)

AUX_SOURCE_DIRECTORY(./proto CLIENT_PROJECT_SRCS)
AUX_SOURCE_DIRECTORY(./framework/lib  CLIENT_PROJECT_SRCS)
ADD_EXECUTABLE(client ./client/client.cpp ${CLIENT_PROJECT_SRCS})
ADD_DEPENDENCIES(client 3rd_lib)

TARGET_LINK_LIBRARIES(client ${CLIENT_LIBRARIES})
SET(EXECUTABLE_OUTPUT_PATH ./bin)

#########################################################################
#########################################################################

PROJECT(send_msg)

add_definitions(-DSIGNED_RIGHT_SHIFT_IS)
add_definitions(-DARITHMETIC_RIGHT_SHIFT)
add_definitions(-DHAVE_NETINET_IN_H)
add_definitions(-DHAVE_INTTYPES_H)
add_definitions(-DHAVE_NETDB_H)

INCLUDE_DIRECTORIES(${THIRD_PARTY_SRC})
INCLUDE_DIRECTORIES(.)

SET(SEND_CLIENT_LIBRARIES
libpthread.so
libdl.so
librt.so

${THIRD_PARTY_LIB}/libthrift.a
${THIRD_PARTY_LIB}/libglog.a
${THIRD_PARTY_LIB}/libprotobuf.a
${THIRD_PARTY_LIB}/libtcmalloc.a
${THIRD_PARTY_LIB}/libhiredis.a
${THIRD_PARTY_LIB}/libboost_regex.a
)

AUX_SOURCE_DIRECTORY(./thrift/gen SEND_PROJECT_SRCS)
AUX_SOURCE_DIRECTORY(./proto SEND_PROJECT_SRCS)
AUX_SOURCE_DIRECTORY(./framework/lib SEND_PROJECT_SRCS)
ADD_EXECUTABLE(send_msg ./client/send_msg.cpp ${SEND_PROJECT_SRCS})
ADD_DEPENDENCIES(send_msg 3rd_lib)

TARGET_LINK_LIBRARIES(send_msg ${SEND_CLIENT_LIBRARIES})
SET(EXECUTABLE_OUTPUT_PATH ./bin)

