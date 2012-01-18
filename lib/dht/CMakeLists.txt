add_library(abdht SHARED
    data.h
    data.cpp
    datakey.h
    datakey.cpp
    datastring.h
    datastring.cpp
    dht.h
    dht.cpp
    messages.h
    messages.cpp
    storage.h
    storage.cpp
    )
SET(PFLIBS ${PFLIBS} abdht)
