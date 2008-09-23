PROJECT(peerfuse)
cmake_minimum_required(VERSION 2.6)

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
		lib/dht/chimera_messages.cpp
		lib/dht/chimera.cpp
		)
SET(PFLIBS ${PFLIBS} dht)

add_library(files
		lib/files/dir_entry.cpp
		lib/files/file_entry.cpp
		lib/files/file_chunk.cpp
#		lib/files/file_content_base.cpp
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

IF(NOT PF_SERVER_MODE)
#	add_library(fuse
#			lib/fuse/fs_utils.cpp
#			lib/fuse/chmod.cpp
#			lib/fuse/chown.cpp
#			lib/fuse/create.cpp
#			lib/fuse/flush.cpp
#			lib/fuse/getattr.cpp
#			lib/fuse/mkdir.cpp
#			lib/fuse/read.cpp
#			lib/fuse/readdir.cpp
#			lib/fuse/rename.cpp
#			lib/fuse/rmdir.cpp
#			lib/fuse/truncate.cpp
#			lib/fuse/unlink.cpp
#			lib/fuse/utimens.cpp
#			lib/fuse/write.cpp
#			)
#	SET(PFLIBS ${PFLIBS} fuse)

ELSE(NOT PF_SERVER_MODE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPF_SERVER_MODE")
ENDIF(NOT PF_SERVER_MODE)

SET(BIN_NAME peerfuse)
ADD_EXECUTABLE(${BIN_NAME}
		peerfuse/tree.cpp
		peerfuse/hdd.cpp
		peerfuse/application.cpp
		peerfuse/environment.cpp
		peerfuse/file_content.cpp
		peerfuse/content_list.cpp
		peerfuse/peerfuse.cpp
		peerfuse/pf_fuse.cpp
		peerfuse/file_entry.cpp
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

## Fuse package, when server mode is disabled
IF(NOT PF_SERVER_MODE)
	INCLUDE(FindFuse.cmake)
	IF(NOT FUSE_FOUND)
		MESSAGE(FATAL_ERROR "Unable to find Fuse library")
	ELSE(NOT FUSE_FOUND)
		MESSAGE(STATUS "Found Fuse: ${FUSE_LINK_DIR} ${FUSE_LDFLAGS}")

		## Check the version
		INCLUDE(CheckLibraryExists)
		CHECK_LIBRARY_EXISTS(fuse fuse_main_real_compat25 "" HAVE_FUSE26)
		IF(NOT HAVE_FUSE26)
			MESSAGE(FATAL_ERROR "The Fuse library installed isn't recent enough. Please install libfuse-2.6 or higher.")
		ENDIF(NOT HAVE_FUSE26)
	ENDIF(NOT FUSE_FOUND)
ENDIF(NOT PF_SERVER_MODE)

## LibCurl
SET(CURL_FIND_REQUIRED TRUE)
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
	MESSAGE(STATUS "Found Curl: ${CURL_LIBRARIES}")
ELSE(CURL_FOUND)
	MESSAGE(FATAL_ERROR "Unable to find Curl library")
ENDIF(CURL_FOUND)

## LibXmlRPC++
SET(XMLRPCXX_FIND_REQUIRED TRUE)
INCLUDE(UsePkgConfig)
PKGCONFIG("libxmlrpc++" XMLRPCXX_INCLUDE_DIR XMLRPCXX_LINK_DIR
	        XMLRPCXX_LDFLAGS XMLRPCXX_CFLAGS)
IF(XMLRPCXX_INCLUDE_DIR OR XMLRPCXX_LINK_DIR OR XMLRPCXX_LDFLAGS OR XMLRPCXX_CFLAGS)
  SET(XMLRPCXX_FOUND TRUE)
ELSE(XMLRPCXX_INCLUDE_DIR OR XMLRPCXX_LINK_DIR OR XMLRPCXX_LDFLAGS OR XMLRPCXX_CFLAGS)
  SET(XMLRPCXX_FOUND FALSE)
ENDIF(XMLRPCXX_INCLUDE_DIR OR XMLRPCXX_LINK_DIR OR XMLRPCXX_LDFLAGS OR XMLRPCXX_CFLAGS)
IF(XMLRPCXX_FOUND)
	MESSAGE(STATUS "Found XmlRPC++: ${XMLRPCXX_LINK_DIR} ${XMLRPCXX_LDFLAGS}")
ELSE(XMLRPCXX_FOUND)
	MESSAGE(FATAL_ERROR "Unable to find XmlRPC++ library")
ENDIF(XMLRPCXX_FOUND)


INCLUDE_DIRECTORIES(${BIN_NAME} ${FUSE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS} ${XMLRPCXX_INCLUDE_DIR}
	"${BIN_NAME}/" "lib/" "lib/fuse" "lib/jobs" "lib/ssl")
LINK_DIRECTORIES(${LIBS_DIR} ${FUSE_LINK_DIR} ${XMLRPCXX_LINK_DIR})
TARGET_LINK_LIBRARIES(${BIN_NAME} "-lpthread -lstdc++" ${FUSE_LDFLAGS} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES} ${XMLRPCXX_LDFLAGS})


SET(CMAKE_BUILD_TYPE ${BUILD})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FUSE_CFLAGS} ${XMLRPCXX_CFLAGS} -DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Werror -Wall -Wextra -Wno-unused-parameter -Wconversion")
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
        DESTINATION doc/peerfuse
        PATTERN ".svn" EXCLUDE)
