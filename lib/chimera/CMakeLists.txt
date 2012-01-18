add_library(abchimera SHARED
    check_leafset_job.h
    check_leafset_job.cpp
    chimera.h
    chimera.cpp
    messages.h
    messages.cpp
    routing.h
    routing.cpp
    leafset.h
    leafset.cpp
    routing_table.h
    routing_table.cpp
    )
SET(PFLIBS ${PFLIBS} abchimera)
