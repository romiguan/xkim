PROJECT(THRIFT-ZOOKEEPER-LIB)

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)

#INCLUDE_DIRECTORIES(/usr/local/include/zookeeper/)

SET(CMAKE_CXX_FLAGS_DEBUG   "$ENV{CXXFLAGS} -g -O0 -Wall -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -W -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS}    -O2 -Wall -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -W -D_FILE_OFFSET_BITS=64 -D_LARGE_FILE")

IF (CMAKE_BUILD_TYPE STREQUAL "Debug")
    MESSAGE(STATUS "CPP Debug Mode")
    MESSAGE(STATUS "Flags:" ${CMAKE_CXX_FLAGS_DEBUG} )
ELSE (CMAKE_BUILD_TYPE STREQUAL "Release")
    MESSAGE(STATUS "CPP Release Mode")
    MESSAGE(STATUS "Flags:" ${CMAKE_CXX_FLAGS_RELEASE})
ENDIF (CMAKE_BUILD_TYPE STREQUAL "Debug")

SET(EXPORT_H
	conn_pool_factory.h
	conn_pool_factory.inl
	conn_pool.h
	conn_pool.inl
	defines.h
	exception.h
	linked_list.h
	lock.h
	object_life_cycle_manager.h
	pooled_client.h
	pooled_client.inl
	server_cluster.h
	singleton_adapter.h
	string_algo.h
	string_algo.inl
	thread_base.h
	thrift_client.h
	thrift_client.inl
	thrift_proxy.h
	thrift_server.h
	unsafe_wait_list.h
    update_task_manager.h
    wait_list.h
    watcher_action.h
    zookeeper_client.h
)

SET(EXPORT_JSON_H
    libjson/autolink.h
    libjson/config.h
    libjson/features.h
    libjson/forwards.h
    libjson/json.h
    libjson/reader.h
    libjson/value.h
    libjson/writer.h
)

SET(LIBRARY_SRC
    watcher_action.cc 
    string_algo.cc 
    zookeeper_client.cc 
    update_task_manager.cc 
    thread_base.cc
    object_life_cycle_manager.cc
)
#gen-cpp/SharedService.cpp
#gen-cpp/shared_types.cpp 
#gen-cpp/tutorial_types.cpp
#gen-cpp/Calculator.cpp
#generated/zookeeper.jute.c)

#LINK_DIRECTORIES(/usr/lib/boost)

ADD_LIBRARY(thriftzookeeper.shared SHARED ${LIBRARY_SRC})

ADD_LIBRARY(thriftzookeeper.static STATIC ${LIBRARY_SRC})

SET_TARGET_PROPERTIES(thriftzookeeper.shared PROPERTIES OUTPUT_NAME "thriftzookeeper")
SET_TARGET_PROPERTIES(thriftzookeeper.shared PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET_TARGET_PROPERTIES(thriftzookeeper.static PROPERTIES OUTPUT_NAME "thriftzookeeper")
SET_TARGET_PROPERTIES(thriftzookeeper.static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

INSTALL(FILES ${EXPORT_H} DESTINATION include/thriftzookeeper)
INSTALL(FILES ${EXPORT_JSON_H} DESTINATION include/thriftzookeeper/libjson)

INSTALL(TARGETS thriftzookeeper.shared LIBRARY DESTINATION lib)
INSTALL(TARGETS thriftzookeeper.static ARCHIVE DESTINATION lib)
