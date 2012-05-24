cmake_minimum_required(VERSION 2.6)

project(ProgSkeet)

set(HEADER_FILES
    progskeet.h
    progskeet_private.h
    )

set(SOURCE_FILES
    progskeet_comm.c
    progskeet_ll.c
    progskeet_utils.c
    progskeet_log.c
    )

OPTION(BUILD_PROGSKEET_SHARED "Build the progskeet library as a shared library (dll/so)." ON)

if(BUILD_PROGSKEET_SHARED)
  set(PROGSKEET_LIBRARY_TYPE SHARED)

  if(WIN32)
    add_definitions(-DDLL_EXPORTS)
  endif(WIN32)
else(BUILD_PROGSKEET_SHARED)
  set(PROGSKEET_LIBRARY_TYPE STATIC)
endif(BUILD_PROGSKEET_SHARED)

add_library(progskeet ${PROGSKEET_LIBRARY_TYPE} ${HEADER_FILES} ${SOURCE_FILES})
