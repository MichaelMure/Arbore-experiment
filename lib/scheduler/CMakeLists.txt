add_library(abscheduler SHARED
    job.h
    job.cpp
    scheduler.h
    scheduler.cpp
    scheduler_queue.h
    scheduler_queue.cpp
    )
SET(PFLIBS ${PFLIBS} abscheduler)
