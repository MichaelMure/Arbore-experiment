add_library(util SHARED
    time.h
    time.cpp
    mutex.h
    mutex.cpp
    tools.h
    tools.cpp
    pf_log.h
    pf_log.cpp
    pf_thread.h
    pf_thread.cpp
    key.h
    key.cpp
    session_config.h
    session_config.cpp
    )
SET(PFLIBS ${PFLIBS} util)
