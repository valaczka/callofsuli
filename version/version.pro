include(../common.pri)

TEMPLATE = aux

AppVersionBuild = "$$cat(version_build.num)"

linux {
	if($$AppVersionIncrement) {
		AppVersionBuild = $$num_add($$AppVersionBuild,1)
		write_file(version_build.num, AppVersionBuild)
	}

	lines = "VER_MAJ = $$AppVersionMajor"
	lines += "VER_MIN = $$AppVersionMinor"
	lines += "VER_PAT = $$AppVersionBuild"
	lines += "VERSION = $${AppVersionMajor}.$${AppVersionMinor}.$${AppVersionBuild}"

	write_file(version.pri, lines)

	hlines = "$${LITERAL_HASH}ifndef _VERSION_H_"
	hlines += "$${LITERAL_HASH}define _VERSION_H_"
	hlines += "$${LITERAL_HASH}define VERSION_MAJOR $${AppVersionMajor}"
	hlines += "$${LITERAL_HASH}define VERSION_MINOR $${AppVersionMinor}"
	hlines += "$${LITERAL_HASH}define VERSION_BUILD $${AppVersionBuild}"
	hlines += "$${LITERAL_HASH}define VERSION_FULL \"$${AppVersionMajor}.$${AppVersionMinor}.$${AppVersionBuild}\""
	hlines += "$${LITERAL_HASH}endif"

	write_file(version.h, hlines)

	build_nr.commands = touch $$_PRO_FILE_
	build_nr.depends = FORCE

	QMAKE_EXTRA_TARGETS += build_nr
	POST_TARGETDEPS += build_nr

}
