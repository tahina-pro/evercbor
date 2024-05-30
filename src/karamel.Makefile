ifeq (,$(EVERCBOR_SRC_PATH))
  $(error "EVERCBOR_SRC_PATH must be set to the absolute path of the src/ subdirectory of the EverCBOR repository")
endif

ifeq (,$(KRML_HOME))
  # assuming Everest layout
  KRML_HOME := $(realpath $(EVERCBOR_SRC_PATH)/../../karamel)
  ifeq (,$(KRML_HOME))
    $(error "KRML_HOME must be defined and set to the root directory of the Karamel repository")
  endif
endif

ALREADY_CACHED := C,LowStar,$(ALREADY_CACHED)

INCLUDE_PATHS += $(KRML_HOME)/krmllib $(KRML_HOME)/krmllib/obj
