TEMPLATE = subdirs

include(modules.pri)

SUBDIRS = $$MODULES_LIST

CONFIG += ordered


lines = "// Auto generated" ""

for(m, MODULES_LIST) {
	lines += "Q_IMPORT_PLUGIN(Module$$upper($$str_member($$m, 0, 0))$$str_member($$m, 1, -1))"
}


write_file(staticmodules.h, lines)



