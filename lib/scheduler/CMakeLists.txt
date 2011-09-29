add_library(scheduler SHARED
    scheduler.cpp
    scheduler_queue.cpp
    job.cpp
    )
SET(PFLIBS ${PFLIBS} scheduler)
