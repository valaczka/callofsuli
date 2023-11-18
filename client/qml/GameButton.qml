import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: control

	property real size: 60
	property real fontImageScale: 0.75
	property alias fontImage: fontImage
	property alias tap: tap
	property alias tapAnim: anim

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

		Qaterial.Icon {
			id: fontImage
			anchors.centerIn: parent
			color: Client.Utils.colorSetAlpha("black", 0)
			size: control.size*fontImageScale
			width: control.size*fontImageScale
			height: control.size*fontImageScale

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

		gesturePolicy: TapHandler.ReleaseWithinBounds | TapHandler.WithinBounds
		grabPermissions: PointerHandler.CanTakeOverFromAnything | PointerHandler.CanTakeOverFromHandlersOfSameType | PointerHandler.CanTakeOverFromItems
						 | PointHandler.ApprovesTakeOverByAnything | PointHandler.ApprovesCancellation

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
