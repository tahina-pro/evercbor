all: verify

EVERCBOR_SRC_PATH = $(realpath ../../..)
INCLUDE_PATHS += $(realpath ..) $(realpath ../..)

include $(EVERCBOR_SRC_PATH)/everparse.Makefile
include $(EVERCBOR_SRC_PATH)/steel.Makefile

ALREADY_CACHED := *,-LowParse.SteelST,
INCLUDE_PATHS += $(STEEL_LIB)/c

include $(EVERCBOR_SRC_PATH)/common.Makefile
