ifeq (,$(EVERCBOR_SRC_PATH))
  $(error "EVERCBOR_SRC_PATH must be set to the absolute path of the src/ subdirectory of the EverCBOR repository")
endif

include $(EVERCBOR_SRC_PATH)/karamel.Makefile

ifeq (,$(EVERPARSE_HOME))
#  $(error "EVERPARSE_HOME must be defined and set to the root directory of the EverParse repository")
  # assuming Everest layout
  EVERPARSE_HOME := $(realpath $(EVERCBOR_SRC_PATH)/../../everparse)
endif

ALREADY_CACHED := LowParse,$(ALREADY_CACHED)

INCLUDE_PATHS += $(EVERPARSE_HOME)/src/lowparse
