include(../common.pri)

TEMPLATE = aux

AppVersionBuild = "$$cat(version_build.num)"

if ($$AppVersionIncrement) {
	AppVersionBuild = $$num_add($$AppVersionBuild,1)
	write_file(version_build.num, AppVersionBuild)
}


lines = "VER_MAJ = $$AppVersionMajor"
lines += "VER_MIN = $$AppVersionMinor"
lines += "VER_PAT = $$AppVersionBuild"
lines += "VERSION = $${AppVersionMajor}.$${AppVersionMinor}.$${AppVersionBuild}"

write_file(version.pri, lines)


