SET(lib_test
    ${OPENSSL_LIBRARIES}
    ${CURL_LIBRARIES}
    "-lpthread -lstdc++"
    util
    scheduler
    network
    dht
    files)


ADD_EXECUTABLE(addr_test
               addr_test.cpp)
TARGET_LINK_LIBRARIES(addr_test
                      ${lib_test})
