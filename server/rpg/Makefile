RCC = rcc

MAPDIRS = $(shell find ./map -maxdepth 1 -mindepth 1 -type d)
TILEDIRS = $(shell find ./tileset -maxdepth 1 -mindepth 1 -type d)


define GEN_MAP
echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$(2); \
echo "<qresource>" >>$(2); \
for ff in $(shell find $(1) -mindepth 1 -maxdepth 1 -type f | sed -e 's+^$(1)/++'); do \
	echo "<file>$(1)/$$ff</file>" >>$(2); \
done; \
echo "</qresource>" >>$(2); \
echo "</RCC>" >>$(2); 
endef

all: $(addsuffix .qrc,$(MAPDIRS)) $(addsuffix .qrc,$(TILEDIRS))

cres: $(addsuffix .cres,$(MAPDIRS))
dres: $(addsuffix .dres,$(TILEDIRS))

distclean: clean

clean: 
	$(shell rm -f $(addsuffix .qrc,$(MAPDIRS)))
	$(shell rm -f $(addsuffix .cres,$(MAPDIRS)))
	$(shell rm -f $(addsuffix .qrc,$(TILEDIRS)))
	$(shell rm -f $(addsuffix .dres,$(TILEDIRS)))


%.cres: %.qrc
	$(RCC) -binary $(<F) -o $(@F)
	rm -f $(<F)

%.dres: %.qrc
	$(RCC) -binary $(<F) -o $(@F)
	rm -f $(<F)

%.qrc: %
	$(call GEN_MAP,$<,$(<F).qrc)
