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


Item {
	id: control

	property real size: 60
	property real fontImageScale: 0.75
	property alias fontImage: fontImage
	property alias tap: tap

	property alias color: rect.color
	property alias border: rect.border
	property alias glowVisible: glow.visible

	width: rect.width
	height: rect.height

	signal clicked()

	Rectangle {
		id: rect

		visible: !glow.visible

		width: size
		height: size
		radius: size/2

		anchors.centerIn: parent

		border.width: 1
		border.color: "black"

		color: "white"

		Behavior on color { ColorAnimation { duration: 125 } }

		QFontImage {
			id: fontImage
			anchors.centerIn: parent
			color: "black"
			size: control.size*fontImageScale
			width: control.width
			height: control.height

			Behavior on color { ColorAnimation { duration: 125 } }
		}
	}

	Glow {
		id: glow
		anchors.fill: rect
		source: rect
		color: "black"
		radius: 2
		spread: 0.3
		samples: 5
	}


	TapHandler {
		id: tap
		onTapped: {
			control.clicked()
			anim.start()
		}
	}

	SequentialAnimation {
		id: anim
		running: false
		PropertyAnimation {
			target: control
			property: "scale"
			to: 0.75
			duration: 125
		}
		PropertyAnimation {
			target: control
			property: "scale"
			to: 1.0
			duration: 175
		}
	}
}
