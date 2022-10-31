include(./version.pri)

TEMPLATE = aux

linux: {
	build_nr.commands = cd $$PWD && ./buildnumber $${VER_MAJ} $${VER_MIN} $${VER_MAINTENANCE}
	build_nr.depends = FORCE

	QMAKE_EXTRA_TARGETS += build_nr
	PRE_TARGETDEPS += build_nr

	CONFIG += skip_version
}
