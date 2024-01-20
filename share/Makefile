RCC = rcc

CRES_DIRS = internal terrain character soldiers sound snipers conquest
CRES_FILES = $(addsuffix .cres,$(CRES_DIRS))

all: $(CRES_FILES)

distclean: clean
	cd ../resources && $(MAKE) clean

clean:
	$(RM) -f $(CRES_FILES)


%.cres: ../resources/%.cres
	cp $< $@

../resources/%.cres: 
	cd ../resources && $(MAKE) $(notdir $@)


