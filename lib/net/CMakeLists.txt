add_library(network SHARED
    host.h
    host.cpp
    hosts_list.h
    hosts_list.cpp
    network.h
    network.cpp
    packet.h
    packet.cpp
    packet_arg.h
    packet_handler.h
    packet_type.h
    packet_type.cpp
    packet_type_list.h
    packet_type_list.cpp
    pf_addr.h
    pf_addr.cpp
    )
SET(PFLIBS ${PFLIBS} network)
