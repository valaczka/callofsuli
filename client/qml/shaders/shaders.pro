TEMPLATE = aux

SHADERS = $$PWD/*.frag


QSB_GLSL = 100es,300es,120,150,330
QSB_HLSL = 50
QSB_MSL = 12

qsb_shaders.input = SHADERS
qsb_shaders.output = ${QMAKE_FILE_IN}.qsb
qsb_shaders.commands = $$shell_quote($$QSB) \
	--glsl $$QSB_GLSL \
	--hlsl $$QSB_HLSL \
	--msl $$QSB_MSL \
	-o $$shell_quote(${QMAKE_FILE_OUT}) \
	$$shell_quote(${QMAKE_FILE_IN})



lines = "<!DOCTYPE RCC><RCC version=\"1.0\">"
lines += "<qresource prefix=\"/shaders\">"

flist = $$files($$PWD/*.frag)

for (file, flist): lines += "	<file>$$basename(file).qsb</file>"

lines += "</qresource></RCC>"

write_file($$OUT_PWD/shaders.qrc, lines)


QSB = $$[QT_INSTALL_BINS]/qsb

qsb_shaders.input = SHADERS
qsb_shaders.output = $$OUT_PWD/${QMAKE_FILE_BASE}.frag.qsb
qsb_shaders.commands = $$QSB \
	--glsl $$QSB_GLSL \
	--hlsl $$QSB_HLSL \
	--msl $$QSB_MSL \
	${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}

qsb_shaders.CONFIG += target_predeps no_link
QMAKE_EXTRA_COMPILERS += qsb_shaders


