import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import "Style"

BusyIndicator {
	id: control

	height: parent.height
	width: parent.height-16
	running: false
	Material.accent: CosStyle.colorAccentDark
}

