########## DEFINITIONS #################
TARGETS			:= baker set-uid

SUBDIRS			:= $(shell find src -type d)
SOURCES			:=
EXCLUDE_SOURCES	:=
INCLUDES		:= -Ilib
LIBRARIES		:= -Llib -lcore -ludev -pthread
DEFINES			:= -DASIO_STANDALONE
OBJECTS			:=
EXTRA_DEPENDENCIES:= lib

########## OPTIONS #####################
#CXX				:= clang++
CXXFLAGS		:= -std=c++14 -O0 -g -Wall

include lib/makefile_tgt.mk
