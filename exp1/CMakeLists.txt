cmake_minimum_required(VERSION 2.8.9)

project(exp1)
 
# -O2 will make the race condition difficult to manifest. but it's there!
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -g -Wall -Wextra -pthread")

# activate with cmake . -DCMAKE_ENV=aarch64
if(CMAKE_ENV STREQUAL "aarch64")
  SET( CMAKE_C_COMPILER "aarch64-linux-gnu-gcc" )
  SET( CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2" )  # for reading disassembly
else ()
  SET( CMAKE_C_COMPILER "gcc" )
endif(CMAKE_ENV STREQUAL "aarch64")


include_directories(
  ./
)

add_definitions(-DDEBUG)

file(GLOB HEADERS
"./*.h"
)

file(GLOB COMMON_SOURCES
  ${HEADERS} 
  measure.c 
 )

# ------------------------------------- #
# all executables  
# ------------------------------------- #
add_executable(counter
  counter.c
  ${COMMON_SOURCES}
)        
# ------------------------------------- #
add_executable(counter-nolock
  counter-nolock.c
  ${COMMON_SOURCES}
)        
# ------------------------------------- #
