add_library(dht SHARED
    check_leafset_job.h
    check_leafset_job.cpp
    chimera.h
    chimera.cpp
    chimera_messages.h
    chimera_messages.cpp
    chimera_routing.h
    chimera_routing.cpp
    dolr.h
    dolr.cpp
    leafset.h
    leafset.cpp
    routing_table.h
    routing_table.cpp
    )
SET(PFLIBS ${PFLIBS} dht)
