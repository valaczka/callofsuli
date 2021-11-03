import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Row {
	id: control

	width: 200

	property alias icon: fontImage.icon
	property color color: CosStyle.colorPrimary

	property alias level: trophy.level
	property alias deathmatch: trophy.isDeathmatch

	property string labelFormat: "%1"

	property alias from: bar.from
	property alias to: bar.to
	property alias value: bar.value

	property real imageSize: 30
	property real labelWidth: CosStyle.pixelSize*5.5
	property alias barWidth: bar.width

	QFontImage {
		id: fontImage
		visible: icon != ""
		icon: ""
		size: imageSize
		color: control.color
		anchors.verticalCenter: parent.verticalCenter
		width: labelWidth

		opacity: bar.value > 0 ? 1.0 : 0.3
	}

	QTrophyImage {
		id: trophy
		visible: level > 0
		level: -1
		isDeathmatch: false
		anchors.verticalCenter: parent.verticalCenter
		height: imageSize*0.7
		width: labelWidth
		opacity: bar.value > 0 ? 1.0 : 0.3
	}

	ProgressBar {
		id: bar
		width: control.width-2*(lblNum.width+10)

		from: 0
		to: 1
		value: 0

		Material.accent: control.color

		anchors.verticalCenter: parent.verticalCenter

		Behavior on value {
			NumberAnimation { duration: 450; easing.type: Easing.OutQuad }
		}
	}

	QLabel {
		id: lblNum
		width: labelWidth
		font.pixelSize: 18
		font.weight: Font.DemiBold
		text: labelFormat.arg(Math.floor(bar.value).toLocaleString())
		color: control.color
		anchors.verticalCenter: parent.verticalCenter
		horizontalAlignment: Label.AlignHCenter
		opacity: bar.value > 0 ? 1.0 : 0.3
	}
}
