add_library(util SHARED
    key.h
    key.cpp
    mutex.h
    mutex.cpp
    pf_config.h
    pf_config.cpp
    pf_exception.h
    pf_log.h
    pf_log.cpp
    pf_thread.h
    pf_thread.cpp
    pf_types.h
    session_config.h
    session_config.cpp
    time.h
    time.cpp
    tools.h
    tools.cpp
    )
SET(PFLIBS ${PFLIBS} util)
