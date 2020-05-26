import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: mainRow.implicitHeight+padding
	implicitWidth: mainRow.implicitWidth+2*padding

	height: implicitHeight
	width: Math.max(implicitWidth, parent.width)

	property int padding: Math.min(parent.height*0.1, 30)

	property alias title: labelTitle.text
	property alias color: labelTitle.color
	property alias icon: mainIcon.icon

	DropShadow {
		anchors.fill: mainRow
		horizontalOffset: 2
		verticalOffset: 2
		radius: 2
		samples: 3
		source: mainRow
		visible: true
	}

	Row {
		id: mainRow

		anchors.horizontalCenter: parent.horizontalCenter

		y: item.padding

		spacing: 10

		QFontImage {
			id: mainIcon

			anchors.verticalCenter: parent.verticalCenter

			color: labelTitle.color

			width: CosStyle.pixelSize*2.0
			height: CosStyle.pixelSize*2.0
			size: CosStyle.pixelSize*2.0
		}

		QLabel {
			id: labelTitle
			anchors.verticalCenter: parent.verticalCenter
			color: CosStyle.colorPrimary
			/*font.capitalization: Font.AllUppercase
			font.weight: Font.DemiBold*/
			font.pixelSize: CosStyle.pixelSize*1.5
			font.family: "HVD Peace"

			width: Math.min(implicitWidth, item.width-mainIcon.width-mainRow.spacing-2*item.padding)

			elide: (item.width-mainIcon.width-mainRow.spacing-2*item.padding) > implicitWidth ?
					   Text.ElideRight : Text.ElideNone
		}
	}
}
