# When adding files here, remember to add "jobs" as a subdir in lib/CMakeList.txt

add_library(jobs SHARED
    )
SET(PFLIBS ${PFLIBS} jobs)
