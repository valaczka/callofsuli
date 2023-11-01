TEMPLATE = aux

include(../common.pri)
include(../version/version.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)
isEmpty(LddPath): error(Missing LddPath)

BinFile = callofsuli-server

CQtTargetDir = CallOfSuli-Server.AppDir

win32 {
	lines = "$${LITERAL_HASH}define COSversion = \"$${VERSION}\""
	lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_server_$${VERSION}_install\""
	lines += $$cat(../server/deploy/CallOfSuli.iss, blob)

	write_file($$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss, lines)

	message(Create InnoSetup: $$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss)

	BinFile = callofsuli-server.exe
}


win32: LddBinFile = $${CQtTargetDir}/usr/$${BinFile}
else: LddBinFile = $${CQtTargetDir}/usr/bin/$${BinFile}

win32: LddLibDir = $${CQtTargetDir}/
else: LddLibDir = $${CQtTargetDir}/usr/lib

win32: extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir}/usr -bin ../$${BinFile} \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml \
			-qmlDir $$PWD/../lib/Qaterial/qml/Qaterial \
			-qmlDir $$PWD/../lib/qml-box2d ; \
			test -d $${CQtTargetDir}/usr/share || mkdir $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}/usr ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}($${LddPath} $${LddBinFile} | \
				grep \"=>\" | grep -i -v \"WINDOWS/SYSTEM32\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done

else: extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir}/usr -bin ../$${BinFile} \
			-libDir ../lib  -libDir ../lib/QtService/lib \
			-qmake $$QMAKE_QMAKE ;\
			test -d $${CQtTargetDir}/usr/share || mkdir $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}/usr ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}($${LddPath} $${LddBinFile} | \
				grep \"=>\" | grep -i -v \"WINDOWS/SYSTEM32\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done ; \
			ln -s usr/$${BinFile}.sh $${CQtTargetDir}/AppRun ; \
			cp $$PWD/../server/deploy/cos_server.png $${CQtTargetDir}/callofsuli-server.png; \
			ln -s callofsuli-server.png $${CQtTargetDir}/.DirIcon ; \
			cp $$PWD/../server/deploy/callofsuli-server.desktop $${CQtTargetDir}



extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


