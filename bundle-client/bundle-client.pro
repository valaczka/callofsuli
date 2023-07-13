TEMPLATE = aux

include(../common.pri)
include(../version/version.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)
isEmpty(LddPath): error(Missing LddPath)

BinFile = callofsuli

win32: CQtTargetDir = .
else: CQtTargetDir = CallOfSuli.AppDir

win32 {
	lines = "$${LITERAL_HASH}define COSversion = \"$${VERSION}\""
	lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_$${VERSION}_install\""
	lines += $$cat(../client/deploy/CallOfSuli.iss, blob)

	write_file($$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss, lines)

	message(Create InnoSetup: $$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss)

	BinFile = callofsuli.exe
}


win32: LibExtension = dll
else: LibExtension = so

win32: LddBinFile = $${CQtTargetDir}/$${BinFile}
else: LddBinFile = $${CQtTargetDir}/usr/bin/$${BinFile}

win32: LddLibDir = $${CQtTargetDir}/usr
else: LddLibDir = $${CQtTargetDir}/usr/lib

win32: extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir}/usr -bin ../$${BinFile} \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter,QOlm \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml ; \
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
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter,QOlm \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml ; \
			test -d $${CQtTargetDir}/usr/share || mkdir $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}/usr ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}($${LddPath} $${LddBinFile} | \
				grep \"=>\" | grep -i -v \"WINDOWS/SYSTEM32\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done ; \
			ln -s usr/$${BinFile}.sh $${CQtTargetDir}/AppRun ; \
			cp $$PWD/../resources/internal/img/cos.png $${CQtTargetDir}/callofsuli.png; \
			ln -s callofsuli.png $${CQtTargetDir}/.DirIcon ; \
			cat $$PWD/../client/deploy/callofsuli.desktop >$${CQtTargetDir}/callofsuli.desktop ; \
			echo "X-AppImage-Version=$${VERSION}" >>$${CQtTargetDir}/callofsuli.desktop ; \
			cp $$PWD/../client/deploy/appimageupdatetool-x86_64.AppImage $${CQtTargetDir}/usr/bin/



extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


