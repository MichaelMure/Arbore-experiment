PROJECT(peerfuse)
cmake_minimum_required(VERSION 2.6)
#SET(BUILD "Debug")
SET(CMAKE_BUILD_TYPE ${BUILD})

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

########################### Library ###################
SUBDIRS (lib)

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
