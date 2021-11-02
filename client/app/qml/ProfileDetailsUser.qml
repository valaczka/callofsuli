import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"

Column {
	id: control
	spacing: 10

	topPadding: 20
	bottomPadding: 20

	property alias image: imgRank.source
	property alias username: labelName.text
	property alias rankname: labelRank.text

	Row {
		id: row
		anchors.horizontalCenter: parent.horizontalCenter

		spacing: 15

		Image {
			id: imgRank
			source: ""

			width: 75
			height: 75

			fillMode: Image.PreserveAspectFit

			anchors.verticalCenter: parent.verticalCenter
		}

		QLabel {
			id: labelName
			font.pixelSize: CosStyle.pixelSize*1.7
			font.weight: Font.Normal
			color: CosStyle.colorAccentLight

			anchors.verticalCenter: parent.verticalCenter

			elide: Text.ElideRight
			width: Math.min(implicitWidth, control.width-row.spacing-imgRank.width)
		}
	}

	QLabel {
		id: labelRank
		anchors.horizontalCenter: parent.horizontalCenter
		font.pixelSize: CosStyle.pixelSize*1.3
		font.weight: Font.Normal
		color: CosStyle.colorAccentLighter

		horizontalAlignment: Text.AlignHCenter

		elide: Text.ElideRight
		width: Math.min(implicitWidth, control.width)

		bottomPadding: 35
	}

}
