add_library(network SHARED
    host.h
    host.cpp
    hosts_list.h
    hosts_list.cpp
    job_handle_packet.h
    job_handle_packet.cpp
    job_resend_packet.h
    job_resend_packet.cpp
    network.h
    network.cpp
    packet.h
    packet.cpp
    packet_arg.h
    packet_handler.h
    packet_handler.cpp
    packet_type.h
    packet_type.cpp
    packet_type_list.h
    packet_type_list.cpp
    pf_addr.h
    pf_addr.cpp
    )
SET(PFLIBS ${PFLIBS} network)
