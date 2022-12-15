TEMPLATE = aux

include(../common.pri)
include(../version/version.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)
isEmpty(LddPath): error(Missing LddPath)

BinFile = callofsuli

win32 {
	lines = "$${LITERAL_HASH}define COSversion = \"$${VERSION}\""
	lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_$${VERSION}_install\""
	lines += $$cat(../client/deploy/CallOfSuli.iss, blob)

	write_file($$OUT_PWD/$${CQtTargetDir}/CallOfSuli.iss, lines)

	message(Create InnoSetup: $$OUT_PWD/$${CQtTargetDir}/CallOfSuli.iss)

	BinFile = callofsuli.exe
}


win32: LddBinFile = $${CQtTargetDir}/$${BinFile}
else: LddBinFile = $${CQtTargetDir}/bin/$${BinFile}

win32: LddLibDir = $${CQtTargetDir}/
else: LddLibDir = $${CQtTargetDir}/lib

extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir} -bin ../$${BinFile} \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$PWD/../client/qml \
			-qmlDir $$PWD/../lib/qaterial/Qaterial-1.4.6/qml/Qaterial ; \
			test -d $${CQtTargetDir}/share || mkdir $${CQtTargetDir}/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir} ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}($${LddPath} $${LddBinFile} | \
				grep \"=>\" | grep -i -v \"WINDOWS/SYSTEM32\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done




extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


