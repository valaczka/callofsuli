#
# Call of Suli
#


#########################################################################
### MAIN SETTINGS
#########################################################################

# App Version

AppVersionMajor = 3
AppVersionMinor = 5

# Automatic version increment (build)

AppVersionIncrement = true



#########################################################################
### BUILD SETTINGS
#########################################################################

# Compile assets into binary

StaticAssets = false

# Android

AndroidVersionCode = 41


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




