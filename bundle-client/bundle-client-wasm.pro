TEMPLATE = aux

RCC = rcc
TARGET_QRC = wasm.qrc
TARGET_RCC = wasm.rcc
SOURCE_DIR = html



lines = "<!DOCTYPE RCC><RCC version=\"1.0\">"
lines += "<qresource prefix=\"/\">"

filelist = $$files($$OUT_PWD/../client/html/*)

message($$filelist)

for (ff, filelist) {
	lines += "<file alias=\"$$basename(ff)\">$${ff}</file>"
}

lines += "</qresource></RCC>"

write_file($$OUT_PWD/wasm.qrc, lines)

extralib.commands = $${RCC} -binary $${TARGET_QRC} -o $${TARGET_RCC}

extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


