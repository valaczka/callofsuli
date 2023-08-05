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



WasmRccFiles = $$files($$PWD/../../share/*.cres)
WasmRccFiles += \
	$$PWD/../client/deploy/callofsuli.html \
	$$PWD/../resources/internal/img/cos.png \
	$$PWD/../client/deploy/wasm_resources.json

WasmRcc.commands = $(COPY_FILE) $$shell_path($$WasmRccFiles) $$shell_path($$OUT_PWD/../client/html/)


QMAKE_EXTRA_TARGETS += WasmRcc

extralib.commands = $${RCC} -binary $${TARGET_QRC} -o $${TARGET_RCC}

extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib

PRE_TARGETDEPS = \
	WasmRcc \
	$$extralib.target


