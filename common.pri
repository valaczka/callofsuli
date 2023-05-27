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

# Add --verbose flag to cmake compiler (Qaterial, qml-box2d)

CMakeVerboseOutput = true

# WebAssembly build with qml-box2d

WasmWithBox2D = true

# Android NDK and SKD path from cmake build (Qaterial, qml-box2d)

AndroidNdkPath = ~/usr/android-sdk/ndk/21.3.6528147
AndroidSdkPath = ~/usr/android-sdk
AndroidVersionCode = 32


#########################################################################
# BUNDLE SETTINGS
#########################################################################

# Build creation enabled (linux, win32, mac, wasm)

CreateBundle = false

# CQtDeployer path

win32: CQtDeployerPath = ~/cqtdeployer/CQtDeployer
else: CQtDeployerPath = cqtdeployer

# LDD path

LddPath = ldd




