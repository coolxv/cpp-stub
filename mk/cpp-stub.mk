# cpp-stub Makefile helper.
#
# Usage:
#   CPP_STUB_DIR := third_party/cpp-stub
#   include $(CPP_STUB_DIR)/mk/cpp-stub.mk
#
#   my_test: my_test.cpp
#       $(CXX) -I$(CPP_STUB_INCLUDE) ... -o $@ $<
#       $(CPP_STUB_POSTLINK)
#
# On macOS $(CPP_STUB_POSTLINK) patches the produced binary's __TEXT
# maxprot to rwx and ad-hoc re-signs it so cpp-stub can rewrite code
# pages at runtime. On Linux/Windows it is a no-op, so the same
# Makefile works unchanged across platforms.

# Resolve this file's directory regardless of how the user included it.
_CPP_STUB_MK_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
CPP_STUB_ROOT    ?= $(abspath $(_CPP_STUB_MK_DIR)/..)
CPP_STUB_INCLUDE ?= $(CPP_STUB_ROOT)/src

_CPP_STUB_UNAME := $(shell uname -s 2>/dev/null)

ifeq ($(_CPP_STUB_UNAME),Darwin)
CPP_STUB_INCLUDE += $(CPP_STUB_ROOT)/src_darwin
CPP_STUB_POSTLINK = $(CPP_STUB_ROOT)/tool/macos_enable_stub.sh $@
else ifeq ($(_CPP_STUB_UNAME),Linux)
CPP_STUB_INCLUDE += $(CPP_STUB_ROOT)/src_linux
CPP_STUB_POSTLINK = :
else
# Windows / other: no-op
CPP_STUB_POSTLINK = :
endif
