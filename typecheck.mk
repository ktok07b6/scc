ROOT_DIR=./
TARGET=typecheck

include $(ROOT_DIR)/mk/common.header.mk

VPATH+=
SRCS+=\
	lexer.cpp \
	Reporter.cpp \
	SourceReader.cpp \
	parserUtils.cpp \
	AST.cpp \
	ConstantExpEvaluator.cpp \
	Symbol.cpp \
	SymbolTable.cpp \
	Scanner.cpp \
	SCParser.cpp \
	SCParserAction.cpp \
	TypeCheck.cpp \
	Type.cpp \
	TypeCreator.cpp \
	typecheck_main.cpp

INCS+=./

OPTS=-Wall -O0 -std=c++0x -g
DEFS+=ENABLE_DEBUG
#DEFS+=ENABLE_FUNCLOG

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


