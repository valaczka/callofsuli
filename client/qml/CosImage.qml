import QtQuick 2.7
import QtGraphicalEffects 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: item

	property alias textColor: overed.color
	property alias glowColor: glowed.color
	property alias glowRadius: glowed.radius
	property alias glowSamples: glowed.samples
	property alias glow: glowed.visible
	property alias image: img

	implicitWidth: 209
	implicitHeight: width*27/209

	Image {
		id: img
		anchors.centerIn: parent
		sourceSize.width: parent.width
		sourceSize.height: parent.height
		width: sourceSize.width
		height: sourceSize.height
		fillMode: Image.PreserveAspectFit
		source: "qrc:/callofsuli.svg"
		visible: false
	}

	ColorOverlay {
		id: overed
		anchors.fill: img
		source: img
		color: Qaterial.Colors.cyan400
	}

	Glow {
		id: glowed
		anchors.fill: img
		source: overed
		radius: 8
		samples: 17
		color: Qaterial.Colors.cyan50
		visible: true
	}


}
