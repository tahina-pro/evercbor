ifeq (,$(EVERCBOR_SRC_PATH))
  $(error "EVERCBOR_SRC_PATH must be set to the absolute path of the src/ subdirectory of the EverCBOR repository")
endif

ifeq (,$(STEEL_LIB))
  ifeq (,$(STEEL_HOME))
    STEEL_LIB := $(shell ocamlfind query steel)
    ifeq (,$(STEEL_LIB))
      $(error "Steel should be installed and its lib/ subdirectory should be in ocamlpath; or STEEL_HOME should be defined in the enclosing Makefile as the prefix directory where Steel was installed, or the root directory of its source repository")
    endif
    STEEL_HOME := $(realpath $(STEEL_LIB)/../..)
  else
    STEEL_LIB := $(STEEL_HOME)/lib/steel
  endif
endif
ifeq ($(OS),Windows_NT)
    OCAMLPATH := $(STEEL_LIB);$(OCAMLPATH)
else
    OCAMLPATH := $(STEEL_LIB):$(OCAMLPATH)
endif
export OCAMLPATH

ALREADY_CACHED := Steel,$(ALREADY_CACHED)

INCLUDE_PATHS += $(STEEL_LIB)

FSTAR_OPTIONS += --load_cmxs steel
