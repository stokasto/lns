cmake_minimum_required(VERSION 2.8)

PROJECT(lns)

INCLUDE_DIRECTORIES(include)

ADD_DEFINITIONS(-g -O2 -Wall -fsigned-char -freg-struct-return -msse2)

ADD_SUBDIRECTORY(src)
ADD_SUBDIRECTORY(include)
#ADD_SUBDIRECTORY(tests)
