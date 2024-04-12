RCC = rcc

MAPDIRS = $(shell find -maxdepth 1 -mindepth 1 -type d)

define GEN_MAP
echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$(2); \
echo "<qresource>" >>$(2); \
for ff in $(shell find $(1) -mindepth 1 -maxdepth 2 -type f | sed -e 's+^$(1)/++'); do \
	echo "<file>$(1)/$$ff</file>" >>$(2); \
done; \
echo "</qresource>" >>$(2); \
echo "</RCC>" >>$(2); 
endef

all: $(addsuffix .qrc,$(MAPDIRS))

cres: $(addsuffix .cres,$(MAPDIRS))

distclean: clean

clean: 
	$(shell rm -f $(addsuffix .qrc,$(MAPDIRS)))
	$(shell rm -f $(addsuffix .cres,$(MAPDIRS)))


%.cres: %.qrc
	$(RCC) -binary $< -o $@
	rm -f $<

%.qrc: %
	$(call GEN_MAP,$<,$<.qrc)

%.dres: %.qrc
	$(RCC) -binary $< -o $@
	rm -f $<
