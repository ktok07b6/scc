ROOT_DIR=./
TARGET=scparser_test

include $(ROOT_DIR)/mk/common.header.mk

VPATH+=
SRCS+=\
	lexer.cpp \
	scparser_test.cpp \
	parserUtils.cpp \
	Reporter.cpp \
	SourceReader.cpp \
	Scanner.cpp \
	SCParser.cpp \
	SCParserAction.cpp \
	AST.cpp \
	Symbol.cpp
INCS+=./

# utility.cpp
OPTS=-Wall -O0 -std=c++0x -g
#OPTS+=-fPIC -finstrument-functions
#OPTS+=-pg
DEFS+=ENABLE_DEBUG
# ENABLE_FUNCLOG

ifeq ($(COVERAGE), yes)
OPTS+=-coverage
LDOPTS+=-coverage
endif

CXXOPTS=$(OPTS)
#LDOPTS+= -pg
#LDFLAGS=-L. -ltracer -ldl -lpthread

#PARSE_DEBUG=1
ifeq ($(PARSE_DEBUG), 1)
DEFS+=LOG_MASK="(ERROR_ON|WARN_ON|INFO_ON|DEBUG_ON|PARSE_ON)"
DEFS+=ENABLE_FUNCLOG
endif

include $(ROOT_DIR)/mk/common.footer.mk


#TARGET_SRCS = $(SRCS)
#TARGET_OBJS = $(call objs_from_sources, $(TARGET_SRCS)) $(EXTRA_OBJS)
cov: $(TARGET)
	./scparser_test
	gcov bin/scparser_test/SCParser.gcno -o bin/scparser_test


