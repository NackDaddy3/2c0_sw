rwildcard=$(wildcard $1/$2) $(foreach d,$(wildcard $1/*/.),$(call rwildcard,$(d:/.=),$2))

# 1. Determine the current directory
CURRENT_DIR := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST))))
UNITTESTDIR = tests/unit_tests

export BUILD_DIR    := build_$(CHIP_TYPE)
BUILD_DIR       	:= $(strip $(BUILD_DIR))

export CHIP_TYPE ?=  # 8181_ES == 8181 B-STEP, 8191_A11, 8191_B11, 8188

# 4. formatting
FORMAT_DIRS_RECURSIVE :=
FORMAT_DIRS_RECURSIVE += $(CURRENT_DIR)/include
FORMAT_DIRS_RECURSIVE += $(CURRENT_DIR)/wrappers
FORMAT_DIRS_RECURSIVE += $(CURRENT_DIR)/src
FORMAT_DIRS_RECURSIVE += $(CURRENT_DIR)/tests/unit_tests

FORMAT_DIRS :=

FORMAT_EXTENSIONS = *.c *.h *.inc

# 5. unit tests
unittests: $(UNITTESTDIR)
.PHONY: unittests

$(UNITTESTDIR):
	$(MAKE) -C $@
.PHONY: $(UNITTESTDIR)

unittests_clean:
	$(MAKE) -C $(UNITTESTDIR) clean


######################################################################################################
##										FORMATTING
######################################################################################################
CLANG_FORMAT_APPLY_COMMAND = clang-format --style=file --verbose -i
CLANG_FORMAT_CHECK_COMMAND = clang-format --style=file --dry-run --Werror --verbose

CLANG_FORMAT_DIRS := $(FORMAT_DIRS)
CLANG_FORMAT_DIRS += $(foreach folder,$(FORMAT_DIRS_RECURSIVE),$(call rwildcard,$(folder),.))

CLANG_FORMAT_DIRS := $(dir $(CLANG_FORMAT_DIRS))
CLANG_FORMAT_DIRS := $(foreach folder,$(CLANG_FORMAT_DIRS),$(foreach ext,$(FORMAT_EXTENSIONS),$(if $(wildcard $(folder)$(ext)),$(folder)$(ext))))


format_check: format_check_all

format: format_all

format_all: $(addsuffix -format,$(CLANG_FORMAT_DIRS)) ;@:
ifndef FORMAT_NO_ECHO
	@echo Formatting of source and header files done.
endif
.PHONY: format_all

format_check_all:$(addsuffix -format_check,$(CLANG_FORMAT_DIRS)) ;@:
ifndef FORMAT_NO_ECHO
	@echo Format Checking of source and header files done.
endif
.PHONY: format_check_all

%-format:
	@$(CLANG_FORMAT_PATH)$(CLANG_FORMAT_APPLY_COMMAND) $*

%-format_check:
	@$(CLANG_FORMAT_PATH)$(CLANG_FORMAT_CHECK_COMMAND) $*
