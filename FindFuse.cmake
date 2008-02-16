#
# Set global variable FUSE to TRUE if both fuse lib and include files are found
#


# Look for FUSE library
FIND_LIBRARY(FUSE_LIBRARY_PATH
             NAMES fuse
             PATHS /lib
                   /usr/lib
                   /usr/local/lib)



# Look for FUSE include files
FIND_PATH(FUSE_INCLUDE_PATH
          NAMES fuse.h
          PATHS /usr/include
                /usr/local/include)

IF(FUSE_LIBRARY_PATH AND FUSE_INCLUDE_PATH)
  SET(FUSE_FOUND TRUE)
  SET(FUSE_LIBRARY "-lfuse")
ELSE(FUSE_LIBRARY_PATH AND FUSE_INCLUDE_PATH)
  SET(FUSE_FOUND FALSE)
ENDIF(FUSE_LIBRARY_PATH AND FUSE_INCLUDE_PATH)

