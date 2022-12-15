#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 3
AppVersionMinor = 2

# Automatic version increment (build)lib

AppVersionIncrement = true


#########################################################################
### BUILD SETTINGS
#########################################################################

# Do not build Qaterial, qml-box2d if dynamic library file available ([build_dir]/lib)

SkipLibraryMakeIfExists = true

# Add --verbose flag to cmake compiler (Qaterial, qml-box2d)

CMakeVerboseOutput = false

# WebAssembly build with qml-box2d

WasmWithBox2D = true

# Android NDK and SKD path from cmake build (Qaterial, qml-box2d)

AndroidNdkPath = /home/valaczka/usr/android-sdk/ndk/21.3.6528147
AndroidSdkPath = /home/valaczka/usr/android-sdk

# cmake compiler path

CMakePath = cmake


#########################################################################
# BUNDLE SETTINGS
#########################################################################

# Build creation enabled (linux, win32, mac)

CreateBundle = true

# CQtDeployer path

win32: CQtDeployerPath = ~/cqtdeployer/CQtDeployer zip
else: CQtDeployerPath = cqtdeployer

# Bundle target directory ([build_dir]/bundle/[CQtTargetDir])

CQtTargetDir = CallOfSuli





#########################################################################
#########################################################################
#########################################################################

# Build configuration

wasm: LibQaterialFile = qaterial/libQaterial.so
else:win32: LibQaterialFile = libQaterial.dll
else: LibQaterialFile = libQaterial.so

QaterialBuildShared = ON

wasm: QmlBox2DBuildShared = OFF
else: QmlBox2DBuildShared = ON

wasm: LibQmlBox2DFile = qml-box2d/bin/plugins/Box2D/libqmlbox2d.a
else:win32: LibQmlBox2DFile = qmlbox2d.dll
else: LibQmlBox2DFile = libqmlbox2d.so
