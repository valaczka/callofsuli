TEMPLATE = aux

include(../common.pri)
include(../version/version.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)


lines = "$${LITERAL_HASH}define COSversion = \"$${VERSION}\""
lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_$${VERSION}_install\""
lines += $$cat(../client/deploy/CallOfSuli.iss, blob)

write_file($${CQtTargetDir}/InnoSetup.iss, lines)

message(Create InnoSetup: $${CQtTargetDir}/InnoSetup.iss)


extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir} -bin ../callofsuli \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml \
			-qmlDir $$PWD/../lib/qaterial/Qaterial-1.4.6/qml/Qaterial ; \
			test -d $${CQtTargetDir}/share || mkdir $${CQtTargetDir}/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}



extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
#PRE_TARGETDEPS = $$extralib.target


