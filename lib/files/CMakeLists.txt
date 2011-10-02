add_library(files SHARED
   file_chunk.cpp
   hdd.h
   hdd.cpp
   file_entry.h
   file_entry.cpp
   file_perms.h
   file_chunk_requester_interface.h
   file_chunk_desc.h
   file_chunk_desc.cpp
   file_chunk.h
   file_chunk.cpp
   dir_entry.h
   dir_entry.cpp
   file_perms.h
   )
SET(PFLIBS ${PFLIBS} files)
