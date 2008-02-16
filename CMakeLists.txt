PROJECT(peerfuse)

SET(SRC
	common/hdd.cpp
	common/job.cpp
	common/libconfig.cpp
	common/log.cpp
	common/mutex.cpp
	common/network_base.cpp
	common/pf_dir.cpp
	common/pf_file.cpp
	common/packet_base.cpp
	common/session_config.cpp
	common/scheduler.cpp
	common/tools.cpp

	common/fuse/chmod.cpp
	common/fuse/chown.cpp
	common/fuse/create.cpp
	common/fuse/flush.cpp
	common/fuse/getattr.cpp
	common/fuse/mkdir.cpp
	common/fuse/readdir.cpp
	common/fuse/rmdir.cpp
	common/fuse/unlink.cpp
	common/fuse/utimens.cpp
	)


IF(PF_NET)
	SET(SRC ${SRC}
		peerfuse-net/network.cpp
		peerfuse-net/pfnet.cpp
		peerfuse-net/pfnet_fuse.cpp
		peerfuse-net/cache.cpp
		peerfuse-net/packet.cpp
		peerfuse-net/net_proto.cpp
		peerfuse-net/peer.cpp)
	SET(BIN_NAME peerfuse-net)
ELSE(PF_NET)
	SET(SRC ${SRC}
		peerfuse-lan/network.cpp
		peerfuse-lan/pflan.cpp
		peerfuse-lan/pflan_fuse.cpp
		peerfuse-lan/cache.cpp
		peerfuse-lan/net_proto.cpp
		peerfuse-lan/packet.cpp
		peerfuse-lan/peer.cpp
		peerfuse-lan/job_other_connect.cpp)
	SET(BIN_NAME peerfuse-lan)
ENDIF(PF_NET)


ADD_EXECUTABLE(${BIN_NAME} ${SRC})

INCLUDE(FindFuse.cmake)
IF(NOT FUSE_FOUND)
	MESSAGE(FATAL_ERROR "Unable to find Fuse library")
ELSE(NOT FUSE_FOUND)
	MESSAGE(STATUS "Found Fuse library")
ENDIF(NOT FUSE_FOUND)

INCLUDE_DIRECTORIES(${BIN_NAME} ${FUSE_INCLUDE_PATH} "${BIN_NAME}/" "src/" "common/" "common/fuse")
LINK_DIRECTORIES(${LIBS_DIR} ${FUSE_LIBRARY_PATH})
TARGET_LINK_LIBRARIES(${BIN_NAME} "-lpthread -lstdc++" ${FUSE_LIBRARY})


SET(CMAKE_BUILD_TYPE ${BUILD})
SET(CMAKE_C_FLAGS "-DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Werror -Wall -Wextra -Wno-unused-parameter -Wconversion")
SET(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})
