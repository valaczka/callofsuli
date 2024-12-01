TEMPLATE = aux

include(../common.pri)
include(../version/version.pri)

isEmpty(CQtDeployerPath): error(Missing CQtDeployerPath)

BinFile = callofsuli

win32: CQtTargetDir = .
else: CQtTargetDir = CallOfSuli.AppDir

win32 {
	lines = "$${LITERAL_HASH}define COSversion = \"$${VERSION}\""
	lines += "$${LITERAL_HASH}define COSexe = \"Call_of_Suli_x64_install_unsigned\""
	lines += "$${LITERAL_HASH}define WinMaxVersion = \"0\""
	lines += "$${LITERAL_HASH}define WinMinVersion = \"10.0.17763\""
	lines += $$cat(../client/deploy/CallOfSuli.iss, blob)

	write_file($$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss, lines)

	message(Create InnoSetup: $$OUT_PWD/$${CQtTargetDir}/usr/CallOfSuli.iss)

	BinFile = callofsuli.exe
}


win32: LibExtension = dll
else: LibExtension = so

win32: LddLibDir = $${CQtTargetDir}/usr
else: LddLibDir = $${CQtTargetDir}/usr/lib

QmlDir = $$PWD/../client/qml

win32: extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -dir $${CQtTargetDir}/usr  \
			--qmake $$QMAKE_QMAKE \
			--qmldir $$QmlDir \
			--no-translations --release --plugindir $${CQtTargetDir}/usr/plugins \
			../$${BinFile} ; \
			test -d $${CQtTargetDir}/usr/share || mkdir $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}/usr ; \
			cp ../$${BinFile} $${LddLibDir} ; \
			cp ../lib/*dll $${LddLibDir} ; \
			cp $$PWD/../client/deploy/qt.conf $${LddLibDir} ; \
			ldd.exe $${LddLibDir}/$${BinFile} >./_tmp_dll.txt ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(cat ./_tmp_dll.txt | \
				grep \"=>\" | grep -i -v \"WINDOWS/\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done ; \
			rm ./_tmp_dll.txt ; \
			which libjpeg-8.dll && cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(which libjpeg-8.dll) $${LddLibDir} ; \
			which libssl-3-x64.dll && cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(which libssl-3-x64.dll) $${LddLibDir} ; \
			which libcrypto-3-x64.dll && cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(which libcrypto-3-x64.dll) $${LddLibDir} ; \
			tar cvf $${CQtTargetDir}/../callofsuli-installer.tar $${CQtTargetDir}

else: extralib.commands = echo \"Create bundle...\"; \
			$${CQtDeployerPath} -targetDir $${CQtTargetDir}/usr -bin ../$${BinFile} \
			-libDir ../lib -extraLibs Qaterial,qmlbox2d,QZXing,QtXlsxWriter,QOlm \
			-qmake $$QMAKE_QMAKE \
			-qmlDir $$QmlDir -enablePlugins multimedia ; \
			test -d $${CQtTargetDir}/usr/share || mkdir $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../share/*.cres $${CQtTargetDir}/usr/share ; \
			cp -r $$PWD/../share/OMRChecker $${CQtTargetDir}/usr/share ; \
			cp $$PWD/../LICENSE $${CQtTargetDir}/usr ; \
			for f in $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(ldd $${CQtTargetDir}/usr/bin/$${BinFile} | \
				grep \"=>\" | grep -i -v \"WINDOWS/SYSTEM32\" | sed \"s/^.*=>[ \t]\\(.*\\) (.*$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}/\1/\") ; do \
				echo \"---> $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f\" ; \
				test -f $${LddLibDir}/$${LITERAL_DOLLAR}$${LITERAL_DOLLAR}(basename $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f) || \
					cp $${LITERAL_DOLLAR}$${LITERAL_DOLLAR}f $${LddLibDir} ; \
			done ; \
			ln -s usr/$${BinFile}.sh $${CQtTargetDir}/AppRun ; \
			cp $$PWD/../resources/internal/img/cos.png $${CQtTargetDir}/callofsuli.png; \
			ln -s callofsuli.png $${CQtTargetDir}/.DirIcon ; \
			cat $$PWD/../client/deploy/callofsuli.desktop >$${CQtTargetDir}/hu.piarista.vjp.callofsuli.desktop ; \
			echo "X-AppImage-Version=$${VERSION}" >>$${CQtTargetDir}/hu.piarista.vjp.callofsuli.desktop ; \
			mkdir -p $${CQtTargetDir}/usr/share/applications ; \
			cat $$PWD/../client/deploy/callofsuli.desktop >$${CQtTargetDir}/usr/share/applications/callofsuli.desktop ; \
			mkdir -p $${CQtTargetDir}/usr/share/icons/hicolor/512x512/apps ; \
			cat $$PWD/../resources/internal/img/cos.png >$${CQtTargetDir}/usr/share/icons/hicolor/512x512/apps/callofsuli.png ; \
			mkdir -p $${CQtTargetDir}/usr/share/metainfo/ ; \
			cp $$PWD/../client/deploy/metainfo.xml $${CQtTargetDir}/usr/share/metainfo/hu.piarista.vjp.callofsuli.appdata.xml ; \
			cp $$PWD/../client/deploy/appimageupdatetool-x86_64.AppImage $${CQtTargetDir}/usr/bin/


extralib.target = .bundle
extralib.depends =


QMAKE_EXTRA_TARGETS += extralib
PRE_TARGETDEPS = $$extralib.target


