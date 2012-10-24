ROOT_DIR=../..
TARGET=SourceReaderTest

include $(ROOT_DIR)/mk/common.header.mk

VPATH+=$(ROOT_DIR)
SRCS+=\
	SourceReader.cpp \
	SourceReaderTest.cpp

INCS+=$(ROOT_DIR)

# utility.cpp
OPTS=-Wall -O0 -std=c++0x -g
DEFS+=ENABLE_DEBUG

CXXOPTS=$(OPTS)

include $(ROOT_DIR)/mk/common.footer.mk

