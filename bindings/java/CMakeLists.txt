find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

add_library(javautil SHARED
            libArbore_util_Log.h
            libArbore_util_Log.cpp
            )

TARGET_LINK_LIBRARIES(javautil
                      abutil
                      )
