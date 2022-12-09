import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

BusyIndicator {
	id: control

	property real size: CosStyle.pixelSize*5
	property color color: CosStyle.colorPrimary

	height: size
	width: size
	running: true
	Material.accent: color
	topPadding: 100
	bottomPadding: 100
}
