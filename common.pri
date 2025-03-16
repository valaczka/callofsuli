#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 4
AppVersionMinor = 5

# Automatic version increment (build)

AppVersionIncrement = true



#########################################################################
### BUILD SETTINGS
#########################################################################

# Android

AndroidVersionCode = 55


#########################################################################
# BUNDLE SETTINGS
#########################################################################


# Build creation enabled (linux, win32, mac, wasm)

CreateBundle = false

# CQtDeployer path

CQtDeployerPath =

# libsodium

LibSodiumInclude =
LibSodiumLibs = -lsodium
LibSodiumDefines =


# Read local configuration

exists(local.pri): include(local.pri)
