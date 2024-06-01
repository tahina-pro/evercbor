all: cbor cddl.do cose.do

EVERCBOR_SRC_PATH = $(CURDIR)

include $(EVERCBOR_SRC_PATH)/pulse.Makefile

SRC_DIRS := cbor cbor/pulse cddl cose

include $(EVERCBOR_SRC_PATH)/common.Makefile

EVERCBOR_LIB_PATH = $(realpath ..)/lib/evercbor

cbor: $(EVERCBOR_LIB_PATH)/evercbor.a

$(EVERCBOR_LIB_PATH):
	mkdir -p $@

$(EVERCBOR_LIB_PATH)/evercbor.a: $(EVERCBOR_LIB_PATH) cbor/pulse/impl/out.do cbor/steel/impl/out.do
	ar cr $@ cbor/steel/impl/out/CBOR.o cbor/pulse/impl/out/CBOR_Pulse.o

cddl.do: cbor verify

cose.do: cbor verify

%.do:
	+$(MAKE) -C $(basename $@)

.PHONY: cbor %.do
