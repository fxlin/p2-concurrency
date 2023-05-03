cmake_minimum_required(VERSION 3.6)

project(exp2-assignment)

## NOTE: toggle below for vtune itt notify instrumentation
SET(USE_VTUNE "1") 
#SET(USE_INIT_ELEMENTS "0")

# -- apply to all configurations --- #
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wall -Wextra -Werror -pthread -O2")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
SET(CMAKE_C_COMPILER "gcc")

find_package(PkgConfig REQUIRED)
pkg_check_modules(deps REQUIRED IMPORTED_TARGET glib-2.0)

include_directories(
  ../exp2/
)

set(COMMON_LIB
    dl
    )
  
if (USE_VTUNE)
    include_directories(
            /opt/intel/vtune_profiler/include   # for vtune instrumentation
    )
    
    ADD_LIBRARY(ittnotify STATIC IMPORTED)
    SET_TARGET_PROPERTIES(ittnotify PROPERTIES IMPORTED_LOCATION /opt/intel/vtune_profiler/lib64/libittnotify.a)
    SET (COMMON_LIB ittnotify ${COMMON_LIB})
    
    add_definitions(-DUSE_VTUNE)    
endif()

#if (USE_INIT_ELEMENTS)
  #add_definitions(-DUSE_INIT_ELEMENTS)
#endif()

add_definitions(-DDEBUG)

file(GLOB HEADERS
"../exp2/*.h"
)

file(GLOB COMMON_SOURCES
  ${HEADERS}
  ../exp2/common.c
  ../exp2/measure.c
  )

# ------------------------------------- #
# all executables  
# ------------------------------------- #

# biglock
add_executable(hashtable-biglock
  hashtable-biglock.c
  ${COMMON_SOURCES}
)

TARGET_LINK_LIBRARIES(hashtable-biglock
        ${COMMON_LIB}
        PkgConfig::deps
        )      

# ------------------------------------- #
