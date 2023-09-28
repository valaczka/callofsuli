#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 3
AppVersionMinor = 3

# Automatic version increment (build)

AppVersionIncrement = false



#########################################################################
### BUILD SETTINGS
#########################################################################

# WebAssembly build with qml-box2d

WasmWithBox2D = true

# Android

AndroidVersionCode = 36


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




