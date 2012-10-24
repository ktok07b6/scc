ROOT_DIR=./
TARGET=scpp

include $(ROOT_DIR)/mk/common.header.mk

VPATH+=
SRCS+=\
	pplexer.cpp \
	AST.cpp \
	Symbol.cpp \
	Scanner.cpp \
	SCParser.cpp \
	SCParserAction.cpp \
	PreProcessor.cpp \
	ConstantExpEvaluator.cpp \
	scpp_main.cpp

INCS+=./

OPTS=-Wall -O0 -std=c++0x -g
DEFS+=ENABLE_DEBUG
DEFS+=ENABLE_FUNCLOG

ifeq ($(COVERAGE), yes)
OPTS+=-coverage
LDOPTS+=-coverage
endif

CXXOPTS=$(OPTS)
#LDOPTS+= -pg
#LDFLAGS=-L. -ltracer -ldl -lpthread

include $(ROOT_DIR)/mk/common.footer.mk

#TARGET_SRCS = $(SRCS)
#TARGET_OBJS = $(call objs_from_sources, $(TARGET_SRCS)) $(EXTRA_OBJS)
cov: $(TARGET)
	./scpp
#	gcov bin/scpp/SCParser.gcno -o bin/scparser_test


