find_package(JNI REQUIRED)
include_directories(${JNI_INCLUDE_DIRS})

add_library(javautil SHARED
            libArbore_util_Log.h
            libArbore_util_Log.cpp
            libArbore_util_Key.h
            libArbore_util_Key.cpp
            )

TARGET_LINK_LIBRARIES(javautil
                      abutil
                      )

add_library(javanetwork SHARED
           libArbore_network_Host.h
           libArbore_network_Host.cpp
	   libArbore_network_Host_List.h
           libArbore_network_Host_List.cpp
           )

TARGET_LINK_LIBRARIES(javanetwork
                      abnetwork
                      abutil
                      abscheduler
                      )

add_library(javascheduler SHARED
            libArbore_scheduler_Scheduler.h
            libArbore_scheduler_Scheduler.cpp
            )

TARGET_LINK_LIBRARIES(javascheduler
                      abscheduler
                      abutil
                      )
