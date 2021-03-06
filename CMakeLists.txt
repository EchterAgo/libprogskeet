cmake_minimum_required(VERSION 2.6)

project(PROGSKEET)

set(
  HEADER_FILES
  progskeet.h
  progskeet_private.h
  )

set(
  SOURCE_FILES
  progskeet_comm.c
  progskeet_ll.c
  progskeet_utils.c
  progskeet_log.c
  )

option(BUILD_PROGSKEET_SHARED "Build the progskeet library as a shared library (dll/so)" ON)

if(BUILD_PROGSKEET_SHARED)
  set(PROGSKEET_LIBRARY_TYPE SHARED)

  if(WIN32)
    add_definitions(-DDLL_EXPORTS)
  endif(WIN32)
else(BUILD_PROGSKEET_SHARED)
  set(PROGSKEET_LIBRARY_TYPE STATIC)
endif(BUILD_PROGSKEET_SHARED)


if(CMAKE_COMPILER_IS_GNUCC)
  option(BUILD_PROGSKEET_FORCE_32BIT "Build the progskeet library as a 32 bit binary" OFF)
  option(BUILD_PROGSKEET_FORCE_64BIT "Build the progskeet library as a 64 bit binary" OFF)

  if(BUILD_PROGSKEET_FORCE_32BIT)
    set(CMAKE_C_FLAGS "-m32")
  else(BUILD_PROGSKEET_FORCE_32BIT)
    if(BUILD_PROGSKEET_FORCE_64BIT)
      set(CMAKE_C_FLAGS "-m64")
    endif(BUILD_PROGSKEET_FORCE_64BIT)
  endif(BUILD_PROGSKEET_FORCE_32BIT)
endif(CMAKE_COMPILER_IS_GNUCC)

add_library(progskeet ${PROGSKEET_LIBRARY_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
