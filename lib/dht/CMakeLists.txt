add_library(dht SHARED
    data.h
    data.cpp
    datakey.h
    datakey.cpp
    datastring.h
    datastring.cpp
    storage.h
    storage.cpp
    )
SET(PFLIBS ${PFLIBS} dht)
