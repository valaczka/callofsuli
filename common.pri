#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 5
AppVersionMinor = 0

# Automatic version increment (build)

AppVersionIncrement = false



#########################################################################
### BUILD SETTINGS
#########################################################################

# Android

AndroidVersionCode = 55


#########################################################################
# BUNDLE SETTINGS
#########################################################################


BackwardCpp = false

# Build creation enabled (linux, win32, mac, wasm)

CreateBundle = false

# CQtDeployer path

CQtDeployerPath =

# libsodium

LibSodiumInclude =
LibSodiumLibs = -lsodium
LibSodiumDefines =

# Extra DLL

ExtraDll =

# Read local configuration

exists(local.pri): include(local.pri)
