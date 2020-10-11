RCC = rcc

SOURCES = $(shell find ../resources -mindepth 2 | sed "s+\.\./resources/++")

all: ../resources/db/default.db callofsuli.rcc

distclean: clean

clean:
	$(RM) callofsuli.rcc


callofsuli.rcc: ../resources/callofsuli.qrc
	$(RCC) -binary $< -o $@
	rm $<

../resources/db/default.db: ../src/resources/default.db
	cp $< $@

../src/resources/default.db:
	cd ../src/resources && $(MAKE) all

../resources/callofsuli.qrc:
	echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$@
	echo "<qresource prefix=\"/\">" >>$@	
	for f in $(SOURCES); do \
		echo "<file>$$f</file>" >>$@ ; \
	done
	echo "</qresource>" >>$@
	echo "</RCC>" >>$@


FORCE:

