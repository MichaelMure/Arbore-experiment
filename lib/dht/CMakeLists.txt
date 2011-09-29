add_library(dht SHARED
    chimera_routing.cpp
    routing_table.cpp
    leafset.cpp
    check_leafset_job.cpp
    dolr.cpp
    chimera_messages.cpp
    chimera.cpp
    )
SET(PFLIBS ${PFLIBS} dht)
