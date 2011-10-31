FILE(GLOB Interfaces *.i)
FILE(GLOB Wrapper *.cxx)

SET_SOURCE_FILES_PROPERTIES(${Interfaces} PROPERTIES CPLUSPLUS ON)
#SET_SOURCE_FILES_PROPERTIES(${Interfaces} PROPERTIES SWIG_FLAGS "-includeall")

SWIG_ADD_MODULE(java_util java ${Interfaces} ${Wrapper})
SWIG_LINK_LIBRARIES(java_util ${JAVA_LIBRARIES})
