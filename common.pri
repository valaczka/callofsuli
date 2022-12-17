#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 3
AppVersionMinor = 2

# Automatic version increment (build)

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

CreateBundle = false

# CQtDeployer path

win32: CQtDeployerPath = ~/cqtdeployer/CQtDeployer
else: CQtDeployerPath = cqtdeployer

# LDD path

LddPath = ldd

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

QmlBox2DBuildShared = OFF

