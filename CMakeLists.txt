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

if(WIN32)
    add_definitions(-DDLL_EXPORTS)
    add_library(progskeet SHARED ${HEADER_FILES} ${SOURCE_FILES})
else()
    add_library(progskeet STATIC ${HEADER_FILES} ${SOURCE_FILES})
endif()
