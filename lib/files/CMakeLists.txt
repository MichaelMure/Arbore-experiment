add_library(files SHARED
    dir_entry.h
    dir_entry.cpp
    file_chunk.h
    file_chunk.cpp
    file_chunk_desc.h
    file_chunk_desc.cpp
    file_chunk_requester_interface.h
    file_content.h
    file_content.cpp
    file_entry.h
    file_entry.cpp
    file_perms.h
    file_perms.cpp
    hdd.h
    hdd.cpp
    )
SET(PFLIBS ${PFLIBS} files)
