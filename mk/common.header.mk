
ifndef TARGET
$(error TARGET undefined) 
endif

BUILD_DIR=$(ROOT_DIR)/bin/$(TARGET)

$(shell mkdir -p $(BUILD_DIR))

#
# Toolchain variables & functions
#
ARFLAGS = rS
CC=$(TOOLCHAIN_PREFIX)gcc
CXX=$(TOOLCHAIN_PREFIX)g++
AR=$(TOOLCHAIN_PREFIX)ar
RANLIB=$(TOOLCHAIN_PREFIX)ranlib

SILENT=
COMPILE.c.cmdline = $(CC) -c $(CFLAGS) -o $(1) $<
COMPILE.cpp.cmdline = $(CXX) -c $(CXXFLAGS) -o $(1) $(2)
GENDEP.c.cmdline = $(CC) -c $(CFLAGS) -MM  -MF $(1) -MT "$(patsubst %.dep,%.o, $(1)) $(1)" $(2)
GENDEP.cpp.cmdline = $(CC) -c $(CXXFLAGS) -MM  -MF $(1) -MT "$(patsubst %.dep,%.o, $(1)) $(1)" $(2)
LINK.o.cmdline = $(CXX) $(2) $(LDFLAGS) -o $(1)
LINK.so.cmdline = $(CXX) $(2) $(LDFLAGS) -shared -o $(1)
ARCHIVE.cmdline = $(AR) $(ARFLAGS) $(1) $(2)

#hooks
VPATH=$(VPATH_HOOK)
INCS=$(INCS_HOOK)
LIB_DIRS=$(LIB_DIRS_HOOK)

SILENT=@
#
# Pattern rules for source files
#
$(BUILD_DIR)/%.o:%.c
	@mkdir -p $(dir $@)
	@echo "COMPILE: "$<
	$(SILENT) $(call COMPILE.c.cmdline,$@,$<)

$(BUILD_DIR)/%.o:%.cpp
	@mkdir -p $(dir $@)
	@echo "COMPILE: "$<
	$(SILENT) $(call COMPILE.cpp.cmdline,$@,$<)

$(BUILD_DIR)/%.o:%.S
	@mkdir -p $(dir $@)
	@echo "COMPILE: "$<
	$(call COMPILE.c.cmdline,$@,$<)

$(BUILD_DIR)/%.dep: %.cpp
	@mkdir -p $(dir $@)
	@echo "GENDEP: "$<
	$(SILENT) $(call GENDEP.cpp.cmdline,$@,$<)

$(BUILD_DIR)/%.dep: %.c
	@mkdir -p $(dir $@)
	@echo "GENDEP: "$<
	$(SILENT) $(call GENDEP.c.cmdline,$@,$<)

#
# Custom functions
#

define objs_from_sources
$(addprefix $(BUILD_DIR)/, \
$(patsubst %.c,%.o, $(filter %.c,$(1))) \
$(patsubst %.cpp,%.o, $(filter %.cpp,$(1))) \
$(patsubst %.S,%.o, $(filter %.S,$(1))) \
)
endef

define deps_from_sources
$(addprefix $(BUILD_DIR)/, \
$(patsubst %.c,%.dep, $(filter %.c,$(1))) \
$(patsubst %.cpp,%.dep, $(filter %.cpp,$(1))) \
)
endef

