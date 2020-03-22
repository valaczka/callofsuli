RCC = /home/valaczka/usr/Qt/5.14.1/gcc_64/bin/rcc

SOURCES = $(shell find ../resources -mindepth 2 | sed "s+\.\./resources/++")

all: callofsuli.rcc

distclean: clean

clean:
	$(RM) callofsuli.rcc


callofsuli.rcc: ../resources/callofsuli.qrc
	$(RCC) -binary $< -o $@
	rm $<

../resources/callofsuli.qrc:
	echo "<!DOCTYPE RCC><RCC version=\"1.0\">" >$@
	echo "<qresource prefix=\"/\">" >>$@	
	for f in $(SOURCES); do \
		echo "<file>$$f</file>" >>$@ ; \
	done
	echo "</qresource>" >>$@
	echo "</RCC>" >>$@


FORCE:

