RCC = rcc

MAPDIRS = $(shell find -maxdepth 1 -mindepth 1 -type d)

define GEN_MAP
echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$(2); \
echo "<qresource prefix=\"/$(1)\">" >>$(2); \
for ff in $(shell find $(1) -mindepth 1 -maxdepth 2 -type f | sed -e 's+^$(1)/++'); do \
	echo "<file>$$ff</file>" >>$(2); \
done; \
echo "</qresource>" >>$(2); \
echo "</RCC>" >>$(2); \
$(RCC) -binary $(2) -o $(3); \
rm -f $(2);
endef


all: $(addsuffix .cres,$(MAPDIRS))

distclean: clean

clean: 
	$(shell rm -f $(addsuffix .cres,$(MAPDIRS)))


%.cres: % 
	$(shell rm -f $</_out.qrc)
	$(call GEN_MAP,$<,$</_out.qrc,$@)
