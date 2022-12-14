AppVersionMajor = 3
AppVersionMinor = 2

AppVersionIncrement = true

CMakeVerboseOutput = false

WasmWithBox2D = true

AndroidNdkPath = /home/valaczka/usr/android-sdk/ndk/21.3.6528147
AndroidSdkPath = /home/valaczka/usr/android-sdk

CMakePath = cmake
CQtDeployerPath = cqtdeployer

CreateBundle = true
CQtTargetDir = CallOfSuli


#######

wasm: LibQaterialFile = qaterial/libQaterial.so
else: LibQaterialFile = libQaterial.so

QaterialBuildShared = ON

wasm: QmlBox2DBuildShared = OFF
else: QmlBox2DBuildShared = ON

wasm: LibQmlBox2DFile = qml-box2d/bin/plugins/Box2D/libqmlbox2d.a
else:win32: LibQmlBox2DFile = libqmlbox2d.dll
else: LibQmlBox2DFile = libqmlbox2d.so
