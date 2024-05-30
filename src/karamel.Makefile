ifeq (,$(KRML_HOME))
  $(error "KRML_HOME must be defined and set to the root directory of the Karamel repository")
endif

ALREADY_CACHED := C,LowStar,$(ALREADY_CACHED)

INCLUDE_PATHS += $(KRML_HOME)/krmllib $(KRML_HOME)/krmllib/obj
