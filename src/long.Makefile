all: cbor/pulse/test.do cose.do

# Test if the two generated CBOR.h have the same number of lines. I
# cannot diff between the two, because Karamel outputs declarations in
# two different orders
cbor-steel: cbor/steel/dist.do cbor/steel/impl.do
	test $$(for f in $$(wc -l cbor/steel/dist/out/CBOR.h) ; do echo $$f ; done | head -n 1) -eq $$(for f in $$(wc -l cbor/steel/impl/out/CBOR.h) ; do echo $$f ; done | head -n 1)

cbor/steel/dist.do: cbor/steel.do

cbor/steel.do: cbor.do

lowparse-steel-array: lowparse/steel/steel_c_array.do lowparse/steel/steel_array.do

lowparse/steel/steel_c_array.do: lowparse/steel.do

lowparse/steel/steel_array.do: lowparse/steel.do

lowparse/steel.do: lowparse.do

cbor/steel/impl.do: cbor/everparse.do cbor/steel.do lowparse-steel-array

cbor/everparse.do: cbor.do lowparse.do

cbor/pulse.do: cbor.do

cbor/pulse/test.do: cbor/pulse.do cbor/steel.do

cddl.do: cbor/pulse.do

cose.do: cddl.do

%.do:
	+$(MAKE) -C $(basename $@)

.PHONY: all %.do lowparse-steel-array cbor-steel
