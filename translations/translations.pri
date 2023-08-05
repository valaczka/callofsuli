LCONVERT_LANGS = hu en
LCONVERT_PATTERNS = qtbase qtmultimedia qtscript qtxmlpatterns

qtPrepareTool(LCONVERT, lconvert)

isEmpty(LCONVERT): error(Missing lconvert)


LCONVERT_OUTPUTS =

for(lang, LCONVERT_LANGS) {
	lang_files = $$PWD/i18n_$${lang}.ts

	for(pat, LCONVERT_PATTERNS) {
		lang_files += $$files($$[QT_INSTALL_TRANSLATIONS]/$${pat}_$${lang}.qm)
	}

	outfile = $$OUT_PWD/qt_$${lang}.qm

	!exists($$outfile): {
		!system("$$LCONVERT -i $$join(lang_files, ' ') -o $$outfile"): error(lconvert error $${lang})
	}

	LCONVERT_OUTPUTS += $$outfile
}

qm_res.files = $$LCONVERT_OUTPUTS
qm_res.base = $$OUT_PWD
qm_res.prefix = "/"



RESOURCES += qm_res