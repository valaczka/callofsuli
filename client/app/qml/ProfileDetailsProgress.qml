import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.15
import COS.Client 1.0
import "."
import "Style"

Column {
	id: control

	property alias value: bar.value
	property alias to: bar.to
	property alias from: bar.from
	property alias color: valueLabel.color
	property string textFormat: "%1"
	property string valueToFormat: textFormat
	property alias valueToVisible: valueTo.visible
	property alias textTo: labelTo.text
	property int duration: 650

	spacing: 0

	QLabel {
		id: valueLabel
		anchors.left: parent.left
		font.pixelSize: CosStyle.pixelSize*1.2
		font.weight: Font.DemiBold
		color: CosStyle.colorPrimaryLighter
		text: control.textFormat.arg(Math.floor(bar.value).toLocaleString())
		//bottomPadding: 5
		//topPadding: 5
	}

	Row {
		id: col

		spacing: 15

		ProgressBar {
			id: bar
			width: control.width-(valueTo.visible ? col.spacing+valueTo.width : 0)

			from: 0
			to: Math.max(value, 1)
			value: 0

			Material.accent: valueLabel.color

			anchors.verticalCenter: parent.verticalCenter

			Behavior on value {
				NumberAnimation { duration: control.duration; easing.type: Easing.OutQuart }
			}
		}

		QLabel {
			id: valueTo
			anchors.verticalCenter: parent.verticalCenter
			font.pixelSize: CosStyle.pixelSize*0.7
			font.weight: Font.DemiBold
			color: valueLabel.color
			text: control.valueToFormat.arg(Math.floor(bar.to).toLocaleString())
		}
	}

	QLabel {
		id: labelTo

		anchors.right: parent.right

		font.pixelSize: CosStyle.pixelSize*0.7
		font.weight: Font.Medium
		color: valueLabel.color
		visible: text != ""
	}
}
