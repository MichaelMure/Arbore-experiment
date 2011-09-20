PROJECT(peerfuse)
cmake_minimum_required(VERSION 2.6)
SET(BUILD "Debug")

add_library(util
		lib/util/dtime.c
                lib/util/pf_config.cpp
		lib/util/mutex.cpp
		lib/util/pf_thread.cpp
		lib/util/tools.cpp
		lib/util/pf_log.cpp
		lib/util/key.cpp
		lib/util/session_config.cpp
		)
SET(PFLIBS ${PFLIBS} util)

add_library(scheduler
		lib/scheduler/scheduler.cpp
		lib/scheduler/scheduler_queue.cpp
		lib/scheduler/job.cpp
		)
SET(PFLIBS ${PFLIBS} scheduler)

add_library(network
		lib/net/pf_addr.cpp
		lib/net/host.cpp
		lib/net/hosts_list.cpp
		lib/net/packet_type.cpp
		lib/net/packet_type_list.cpp
		lib/net/packet.cpp
		lib/net/network.cpp
	   )
SET(PFLIBS ${PFLIBS} network)

add_library(dht
		lib/dht/chimera_routing.cpp
		lib/dht/routing_table.cpp
		lib/dht/leafset.cpp
		lib/dht/check_leafset_job.cpp
		lib/dht/dolr.cpp
		lib/dht/chimera_messages.cpp
		lib/dht/chimera.cpp
		)
SET(PFLIBS ${PFLIBS} dht)

add_library(files
		lib/files/dir_entry.cpp
		lib/files/file_entry.cpp
		lib/files/file_chunk.cpp
		lib/files/file_content.cpp
		lib/files/hdd.cpp
	   )
SET(PFLIBS ${PFLIBS} files)

#add_library(ssl
#		lib/ssl/certificate.cpp
#		lib/ssl/connection.cpp
#		lib/ssl/connection_ssl.cpp
#		lib/ssl/connection_nossl.cpp
#		lib/ssl/crl.cpp
#		lib/ssl/pf_ssl_ssl.cpp
#		lib/ssl/pf_ssl_nossl.cpp
#		lib/ssl/private_key.cpp
#	   )
#SET(PFLIBS ${PFLIBS} ssl)


SET(BIN_NAME peerfuse)
ADD_EXECUTABLE(${BIN_NAME}
		peerfuse/peerfuse.cpp
		peerfuse/application.cpp
		peerfuse/tree.cpp
		peerfuse/environment.cpp
		peerfuse/content_list.cpp
		peerfuse/file_chunk_requester.cpp
		peerfuse/pf_fuse.cpp
		peerfuse/filedist.cpp
		peerfuse/job_new_conn_req.cpp
		peerfuse/job_update_resp_files.cpp
		peerfuse/job_ls_dir.cpp
		peerfuse/job_end_of_ls.cpp
	      )
SET(CONF_NAME peerfuse.conf)

target_link_libraries(${BIN_NAME} ${PFLIBS})

## OpenSSL
SET(OpenSSL_FIND_REQUIRED TRUE)
FIND_PACKAGE(OpenSSL)
IF(NOT OPENSSL_FOUND)
	MESSAGE(FATAL_ERROR "Unable to OpenSSL library")
ENDIF(NOT OPENSSL_FOUND)

## LibCurl
SET(CURL_FIND_REQUIRED TRUE)
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
	MESSAGE(STATUS "Found Curl: ${CURL_LIBRARIES}")
ELSE(CURL_FOUND)
	MESSAGE(FATAL_ERROR "Unable to find Curl library")
ENDIF(CURL_FOUND)


INCLUDE_DIRECTORIES(${BIN_NAME} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS} "${BIN_NAME}/" "lib/")
LINK_DIRECTORIES(${LIBS_DIR} )
TARGET_LINK_LIBRARIES(${BIN_NAME} "-lpthread -lstdc++" ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} )


SET(CMAKE_BUILD_TYPE ${BUILD})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Werror -Wall -Wextra -Wno-unused-parameter -Wconversion")
SET(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

STRING(COMPARE EQUAL ${CMAKE_BUILD_TYPE} "Debug" DEBUG_BUILD)
IF(DEBUG_BUILD)
	MESSAGE(STATUS "We'll perform tests during compilation")
	# Looks a bit like a hack.. should find a better way to do this
	SET(CMAKE_C_FLAGS "${CMAKE_C_COMPILER} ${CMAKE_C_FLAGS}")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_FLAGS}")
	SET(CMAKE_C_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/tools/gcc-wrapper/my-gcc.sh")
	SET(CMAKE_CXX_COMPILER "${CMAKE_CURRENT_SOURCE_DIR}/tools/gcc-wrapper/my-gcc.sh")
ENDIF(DEBUG_BUILD)

MESSAGE(STATUS "Using compiler ${CMAKE_CXX_COMPILER}")
MESSAGE(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

INSTALL(TARGETS ${BIN_NAME}
        DESTINATION bin)
INSTALL(FILES ${CONF_NAME}
        DESTINATION etc/peerfuse)
INSTALL(FILES BUGS COPYING HACKING README ROADMAP TASKS
        DESTINATION doc/peerfuse)
INSTALL(DIRECTORY doc/
        DESTINATION doc/peerfuse)
