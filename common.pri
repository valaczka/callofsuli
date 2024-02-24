#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 3
AppVersionMinor = 7

# Automatic version increment (build)

AppVersionIncrement = false



#########################################################################
### BUILD SETTINGS
#########################################################################

# Android

AndroidVersionCode = 43


#########################################################################
# BUNDLE SETTINGS
#########################################################################


# Build creation enabled (linux, win32, mac, wasm)

CreateBundle = true

# CQtDeployer path

win32: CQtDeployerPath = ~/cqtdeployer/CQtDeployer
else: CQtDeployerPath = cqtdeployer

# LDD path

LddPath = ldd




