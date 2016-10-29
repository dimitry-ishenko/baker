########## DEFINITIONS #################
TARGET		:= baker

SUBDIRS		:= $(shell find src -type d)
SOURCES		:=
INCLUDES	:= -Ilib
LIBRARIES	:= -Llib -lcore -ludev -pthread
DEFINES		:= -DASIO_STANDALONE
OBJECTS		:=

########## OPTIONS #####################
#CXX			:= clang++
CXXFLAGS	:= -std=c++14 -O0 -g -Wall

include lib/makefile_tgt.mk

########## DEPENDENCIES ################
$(TARGET):	libcore.a

libcore.a:
	@echo "Compiling $@"
	@$(MAKE) -C lib
