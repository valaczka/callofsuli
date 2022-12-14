TEMPLATE = aux

include(../common.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)

extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir} -bin ../callofsuli \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml \
			-qmlDir $$PWD/../lib/qaterial/Qaterial-1.4.6/qml/Qaterial ; \
			test -d $${CQtTargetDir}/share || mkdir $${CQtTargetDir}/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/share



extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


