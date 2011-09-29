add_library(network SHARED
    host.cpp
    hosts_list.cpp
    packet_type.cpp
    packet_type_list.cpp
    packet.cpp
    network.cpp
    pf_addr.cpp
    )
SET(PFLIBS ${PFLIBS} network)
