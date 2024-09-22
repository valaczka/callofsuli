#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 4
AppVersionMinor = 3

# Automatic version increment (build)

AppVersionIncrement = false



#########################################################################
### BUILD SETTINGS
#########################################################################

# Android

AndroidVersionCode = 51


#########################################################################
# BUNDLE SETTINGS
#########################################################################


# Build creation enabled (linux, win32, mac, wasm)

CreateBundle = true

# CQtDeployer path

win32: CQtDeployerPath = windeployqt6.exe
else: CQtDeployerPath = ~/bin/CQtDeployer
