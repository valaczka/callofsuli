INCLUDEPATH += $$PWD/qsyncable

HEADERS += \
	$$PWD/qsyncable/qsdiffrunner.h \
	$$PWD/qsyncable/qspatch.h \
	$$PWD/qsyncable/qspatchable.h \
	$$PWD/qsyncable/qslistmodel.h \
	#$$PWD/qsyncable/qsuuid.h \
	$$PWD/qsyncable/priv/qsdiffrunneralgo_p.h \
	$$PWD/qsyncable/priv/qstree.h \
	$$PWD/qsyncable/priv/qstreenode.h \
	$$PWD/qsyncable/qsjsonlistmodel.h \
	$$PWD/qsyncable/QSDiffRunner \
	$$PWD/qsyncable/QSListModel \
	$$PWD/qsyncable/qsyncablefunctions.h \
	#$$PWD/qsyncable/qsyncableqmlwrapper.h \
	$$PWD/qsyncable/priv/qsalgotypes_p.h \
	$$PWD/qsyncable/priv/qsimmutablewrapper_p.h \
	$$PWD/qsyncable/priv/qsfastdiffrunneralgo_p.h \
	$$PWD/qsyncable/qsfastdiffrunner.h

SOURCES += \
	$$PWD/qsyncable/qsdiffrunner.cpp \
	$$PWD/qsyncable/qspatch.cpp \
	$$PWD/qsyncable/qslistmodel.cpp \
	#$$PWD/qsyncable/qsuuid.cpp \
	$$PWD/qsyncable/qsdiffrunneralgo.cpp \
	$$PWD/qsyncable/qstree.cpp \
	$$PWD/qsyncable/qstreenode.cpp \
	$$PWD/qsyncable/qsjsonlistmodel.cpp \
	#$$PWD/qsyncable/qsyncableqmltypes.cpp \
	$$PWD/qsyncable/qsyncablefunctions.cpp \
	#$$PWD/qsyncable/qsyncableqmlwrapper.cpp
