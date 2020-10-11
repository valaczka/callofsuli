RCC = rcc

MAPDIRS = $(shell find -maxdepth 1 -mindepth 1 -type d)


define GEN_MAP
echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$(2); \
echo "<qresource prefix=\"/\">" >>$(2); \
for ff in $(shell find $(1) -mindepth 1 -type f | sed 's+^$(1)/++'); do \
	echo "<file>$$ff</file>" >>$(2); \
done; \
echo "</qresource>" >>$(2); \
echo "</RCC>" >>$(2); \
$(RCC) -binary $(2) -o $(3); \
rm -f $(2);
endef


all: characters

distclean: clean

clean: 
	$(foreach f, $(MAPDIRS), $(shell rm -f $(f).cosc))


characters: clean
	$(foreach f, $(MAPDIRS), $(shell rm -f $(f)/_out.qrc))
	@$(foreach f, $(MAPDIRS), $(call GEN_MAP,$(f),$(f)/_out.qrc,$(f).cosc))

