add_library(util SHARED
    time.cpp
    mutex.cpp
    tools.cpp
    pf_log.cpp
    pf_thread.cpp
    key.cpp
    )
SET(PFLIBS ${PFLIBS} util)
