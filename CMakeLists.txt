PROJECT(peerfuse)
cmake_minimum_required(VERSION 2.6)
#SET(BUILD "Debug")
SET(CMAKE_BUILD_TYPE ${BUILD})
#SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE})


############################# Library ############

SET(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

add_library(util
    lib/util/time.cpp
    lib/util/mutex.cpp
    lib/util/tools.cpp
    lib/util/pf_log.cpp
    lib/util/pf_thread.cpp
    lib/util/key.cpp
    )
SET(PFLIBS ${PFLIBS} util)

add_library(scheduler
    lib/scheduler/scheduler.cpp
    lib/scheduler/scheduler_queue.cpp
    lib/scheduler/job.cpp
    )
SET(PFLIBS ${PFLIBS} scheduler)

add_library(network
    lib/net/host.cpp
    lib/net/hosts_list.cpp
    lib/net/packet_type.cpp
    lib/net/packet_type_list.cpp
    lib/net/packet.cpp
    lib/net/network.cpp
    lib/net/pf_addr.cpp
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
   lib/files/file_chunk.cpp
    )
SET(PFLIBS ${PFLIBS} files)

#add_library(ssl
#   lib/ssl/certificate.cpp
#   lib/ssl/connection.cpp
#   lib/ssl/connection_ssl.cpp
#   lib/ssl/connection_nossl.cpp
#   lib/ssl/crl.cpp
#   lib/ssl/pf_ssl_ssl.cpp
#   lib/ssl/pf_ssl_nossl.cpp
#   lib/ssl/private_key.cpp
#    )
#SET(PFLIBS ${PFLIBS} ssl)

########################### Dependency #################

## OpenSSL
SET(OpenSSL_FIND_REQUIRED TRUE)
FIND_PACKAGE(OpenSSL)
IF(OPENSSL_FOUND)
  MESSAGE(STATUS "Found OpenSSL: ${OPENSSL_LIBRARIES}")
ELSE(OPENSSL_FOUND)
  MESSAGE(FATAL_ERROR "Unable to OpenSSL library")
ENDIF(OPENSSL_FOUND)

## LibCurl
SET(CURL_FIND_REQUIRED TRUE)
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  MESSAGE(STATUS "Found Curl: ${CURL_LIBRARIES}")
ELSE(CURL_FOUND)
  MESSAGE(FATAL_ERROR "Unable to find Curl library")
ENDIF(CURL_FOUND)

########################## Target #####################

SET(BIN_NAME dht_test)
ADD_EXECUTABLE(${BIN_NAME}
    dhttest/main.cpp
    )

INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR}
                    ${CURL_INCLUDE_DIRS}
                    lib/)
LINK_DIRECTORIES(${OPENSSL_LIB_DIR}
                 ${CURL_LIB_DIR}
                 ${LIBRARY_OUTPUT_PATH})
TARGET_LINK_LIBRARIES(${BIN_NAME}
                      "-lpthread -lstdc++"
                      ${OPENSSL_LIBRARIES}
                      ${CURL_LIBRARIES}
                      util
                      scheduler
                      network
                      dht
                      files)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Werror -Wall -Wextra -Wno-unused-parameter -Wconversion")
SET(CMAKE_CXX_FLAGS ${CMAKE_C_FLAGS})

STRING(COMPARE EQUAL "${CMAKE_BUILD_TYPE}" "Debug" DEBUG_BUILD)
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
