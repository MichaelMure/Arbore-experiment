PROJECT(peerfuse)
cmake_minimum_required(VERSION 2.6)

#SET(CMAKE_BUILD_TYPE "Debug")

########################## Target #####################

SET(BIN_NAME dht_test)
ADD_EXECUTABLE(${BIN_NAME}
               dhttest/main.cpp
               )

INCLUDE_DIRECTORIES(lib/)
LINK_DIRECTORIES(${LIBRARY_OUTPUT_PATH})
TARGET_LINK_LIBRARIES(${BIN_NAME}
                      "-lpthread -lstdc++"
                      util
                      scheduler
                      network
                      dht
                      files)

########################### Dependency #################

## OpenSSL
SET(OpenSSL_FIND_REQUIRED TRUE)
FIND_PACKAGE(OpenSSL)
IF(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  LINK_DIRECTORIES(${OPENSSL_LIB_DIR})
  TARGET_LINK_LIBRARIES(${BIN_NAME}
                        ${OPENSSL_LIBRARIES})
ENDIF(OPENSSL_FOUND)

## LibCurl
SET(CURL_FIND_REQUIRED TRUE)
FIND_PACKAGE(CURL)
IF(CURL_FOUND)
  INCLUDE_DIRECTORIES(${CURL_INCLUDE_DIRS})
  LINK_DIRECTORIES(${CURL_LIB_DIR})
  TARGET_LINK_LIBRARIES(${BIN_NAME}
                        ${CURL_LIBRARIES})
ENDIF(CURL_FOUND)

########################### Library ###################

SUBDIRS (lib)

########################### Tests #####################

SUBDIRS (tests)

########################### Bindings ##################

IF(BUILD_BINDINGS)
  MESSAGE(STATUS "Bindings building enabled")
  SUBDIRS(bindings)
ENDIF(BUILD_BINDINGS)

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -ggdb -DDEBUG -D_REENTRANT -D_FILE_OFFSET_BITS=64 -Wextra -Wall -Wcast-align -Wcast-qual -Wconversion -Wdisabled-optimization -Wfloat-equal -Wformat=2 -Wimport -Winit-self -Winline -Winvalid-pch -Wunsafe-loop-optimizations -Wlong-long -Wmissing-format-attribute -Wmissing-include-dirs -Wmissing-noreturn -Wpacked -Wpointer-arith -Wredundant-decls -Wshadow -Wstack-protector -Wswitch-default -Wswitch-enum -Wunused -Wvariadic-macros -Wwrite-strings")
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
