
CFLAGS=$(COPTS) $(addprefix -D,$(DEFS)) $(addprefix -I,$(INCS))
CXXFLAGS=$(CXXOPTS) $(addprefix -D,$(DEFS)) $(addprefix -I,$(INCS))
LDFLAGS=$(LDOPTS) $(addprefix -L,$(LIB_DIRS)) $(addprefix -l,$(LIBS))

TARGET_SRCS = $(SRCS)
TARGET_OBJS = $(call objs_from_sources, $(TARGET_SRCS)) $(EXTRA_OBJS)


all: $(TARGET)


$(TARGET): $(TARGET_OBJS)
	@echo linking ... $@
	$(SILENT) $(call LINK.o.cmdline, $@, $^)
	@echo done.

lexer.cpp: sc.l
	flex -olexer.cpp sc.l

pplexer.cpp: scpp.l
	flex -opplexer.cpp scpp.l

DEPFILES = $(call deps_from_sources, $(TARGET_SRCS))

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILES)
endif

clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)
	rm -f lexer.cpp pplexer.cpp
	rm -f *.gcov

