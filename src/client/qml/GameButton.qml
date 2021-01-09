import QtQuick 2.15
import QtQuick.Controls 2.15
import Bacon2D 1.0
import COS.Client 1.0
import QtQuick.Controls.Material 2.12
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS


Rectangle {
	id: control

	property real size: 60
	property real fontImageScale: 0.75
	property alias fontImage: fontImage
	property alias tap: tap


	signal clicked()

	width: size
	height: size
	radius: size/2

	border.width: 1
	border.color: "black"

	color: "white"

	Behavior on color { ColorAnimation { duration: 75 } }

	QFontImage {
		id: fontImage
		anchors.centerIn: parent
		color: "black"
		size: control.size*fontImageScale
		width: control.width
		height: control.height

		Behavior on color { ColorAnimation { duration: 75 } }
	}

	TapHandler {
		id: tap
		onTapped: control.clicked()
	}
}
