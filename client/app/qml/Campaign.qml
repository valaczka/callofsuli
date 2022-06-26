import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control
	required property string title
	required property string subtitle
	required property var assignment
	required property bool finished

	implicitWidth: 200
	implicitHeight: 200

	width: parent.width
	height: col.height+85+50

	BorderImage {
		width: parent.width/scale
		height: parent.height/scale
		source: "qrc:/internal/img/paper.png"
		border {
			left: 253
			top: 129
			right: 1261-1175
			bottom: 851-737
		}
		scale: 0.4
		transformOrigin: Item.TopLeft
	}

	Column {
		id: col
		x: 50
		width: parent.width-100
		y: 35

		QLabel {
			text: control.title
			width: parent.width
			wrapMode: Text.Wrap
			font.family: "HVD Peace"
			font.pixelSize: CosStyle.pixelSize*1.7
			color: control.finished ? "black" :"saddlebrown"
		}

		QLabel {
			text: control.subtitle
			width: parent.width
			wrapMode: Text.Wrap
			color: control.finished ? "black" :"saddlebrown"
			font.pixelSize: CosStyle.pixelSize*0.8
			font.weight: Font.DemiBold
		}


		Repeater {
			model: control.assignment

			CampaignAssignment {
				required property int index
				separator: index > 0
			}
		}
	}

}
