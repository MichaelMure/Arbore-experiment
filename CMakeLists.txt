PROJECT(peerfuse)

SET(SRC
	common/cache_base.cpp
	common/content_list_base.cpp
	common/download.cpp
	common/dir_entry.cpp
	common/file_chunk.cpp
	common/file_content_base.cpp
	common/fs_utils.cpp
	common/hdd.cpp
	common/pf_config.cpp
	common/pf_log.cpp
	common/mutex.cpp
	common/network_base.cpp
	common/peer_base.cpp
	common/peers_list_base.cpp
	common/pf_filebase.cpp
	common/pf_thread.cpp
	common/packet_base.cpp
	common/session_config.cpp
	common/scheduler.cpp
	common/scheduler_queue.cpp
	common/tools.cpp

	common/jobs/job.cpp
	common/jobs/job_new_connection.cpp
	common/jobs/job_new_conn_req_base.cpp
	common/jobs/job_new_connection_queue.cpp
	common/jobs/job_send_ref_file.cpp
	common/jobs/job_send_chunk.cpp
	common/jobs/job_set_chunk.cpp
	common/jobs/job_set_shared_part.cpp
	common/jobs/job_mkfile.cpp
	common/jobs/job_send_mkfile.cpp

	common/ssl/certificate.cpp
	common/ssl/connection.cpp
	common/ssl/connection_ssl.cpp
	common/ssl/connection_nossl.cpp
	common/ssl/crl.cpp
	common/ssl/pf_ssl_ssl.cpp
	common/ssl/pf_ssl_nossl.cpp
	common/ssl/private_key.cpp
	)

IF(NOT PF_SERVER_MODE)
	SET(SRC ${SRC}
		common/fuse/chmod.cpp
		common/fuse/chown.cpp
		common/fuse/create.cpp
		common/fuse/flush.cpp
		common/fuse/getattr.cpp
		common/fuse/mkdir.cpp
		common/fuse/read.cpp
		common/fuse/readdir.cpp
		common/fuse/rename.cpp
		common/fuse/rmdir.cpp
		common/fuse/truncate.cpp
		common/fuse/unlink.cpp
		common/fuse/utimens.cpp
		common/fuse/write.cpp)
ELSE(NOT PF_SERVER_MODE)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPF_SERVER_MODE")
ENDIF(NOT PF_SERVER_MODE)

IF(PF_NET)
	SET(SRC ${SRC}
		peerfuse-net/application.cpp
		peerfuse-net/environment.cpp
		peerfuse-net/network.cpp
		peerfuse-net/file_content.cpp
		peerfuse-net/content_list.cpp
		peerfuse-net/pfnet.cpp
		peerfuse-net/pfnet_fuse.cpp
		peerfuse-net/cache.cpp
		peerfuse-net/pf_file.cpp
		peerfuse-net/filedist.cpp
		peerfuse-net/job_new_conn_req.cpp
		peerfuse-net/job_update_resp_files.cpp
		peerfuse-net/job_ls_dir.cpp
		peerfuse-net/job_end_of_ls.cpp
		peerfuse-net/packet.cpp
		peerfuse-net/peers_list.cpp
		peerfuse-net/net_proto.cpp
		peerfuse-net/peer.cpp)
	SET(BIN_NAME peerfuse-net)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPF_NET")
	SET(CONF_NAME pfnet.conf)
ELSE(PF_NET)
	SET(SRC ${SRC}
		peerfuse-lan/application.cpp
		peerfuse-lan/environment.cpp
		peerfuse-lan/network.cpp
		peerfuse-lan/file_content.cpp
		peerfuse-lan/content_list.cpp
		peerfuse-lan/pflan.cpp
		peerfuse-lan/pflan_fuse.cpp
		peerfuse-lan/cache.cpp
		peerfuse-lan/net_proto.cpp
		peerfuse-lan/packet.cpp
		peerfuse-lan/peer.cpp
		peerfuse-lan/peers_list.cpp
		peerfuse-lan/job_advertise_file.cpp
		peerfuse-lan/job_send_changes.cpp
		peerfuse-lan/job_set_sharer.cpp
		peerfuse-lan/job_new_conn_req.cpp
		peerfuse-lan/job_other_connect.cpp)
	SET(BIN_NAME peerfuse-lan)
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPF_LAN")
	SET(CONF_NAME pflan.conf)
ENDIF(PF_NET)


ADD_EXECUTABLE(${BIN_NAME} ${SRC})

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


INCLUDE_DIRECTORIES(${BIN_NAME} ${FUSE_INCLUDE_DIR} ${OPENSSL_INCLUDE_DIR} ${CURL_INCLUDE_DIRS}
	"${BIN_NAME}/" "common/" "common/fuse" "common/jobs" "common/ssl")
LINK_DIRECTORIES(${LIBS_DIR} ${FUSE_LINK_DIR})
TARGET_LINK_LIBRARIES(${BIN_NAME} "-lpthread -lstdc++" ${FUSE_LDFLAGS} ${OPENSSL_LIBRARIES} ${CURL_LIBRARIES})


SET(CMAKE_BUILD_TYPE ${BUILD})
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FUSE_CFLAGS} -DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Werror -Wall -Wextra -Wno-unused-parameter -Wconversion")
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
