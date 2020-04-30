import QtQuick 2.12
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	property int imageSize: 40
	property alias imageSource: userImg.source
	property alias imageSourceSize: userImg.sourceSize

	property alias userName: userLabel.text

	property bool isExpanded: false

	property alias image: userImg
	property alias label: userLabel

	property UserDetails userDetails: null


	color: area.containsMouse ? "grey" : "transparent"
	radius: 3

	width: userRow.width+16
	height: userRow.height+8

	scale: area.pressed ? 0.9 : 1.0

	Behavior on scale {
		NumberAnimation { duration: 75 }
	}

	signal clicked()

	Row {
		id: userRow

		anchors.centerIn: parent

		spacing: 10

		Item {
			width: userImg.width
			height: userImg.height

			anchors.verticalCenter: parent.verticalCenter

			Image {
				id: userImg

				source: "image://sql/rank/"+cosClient.userRank+".svg"

				width: control.imageSize
				height: control.imageSize

				fillMode: Image.PreserveAspectFit
			}

			Glow {
				anchors.fill: userImg
				radius: 1
				samples: 3
				color: "yellow"
				source: userImg
				visible: area.containsMouse
			}
		}

		Label {
			id: userLabel
			font.pixelSize: CosStyle.pixelSize*0.9

			text: cosClient.userFirstName+"\n"+cosClient.userLastName

			anchors.verticalCenter: parent.verticalCenter
		}
	}

	MouseArea {
		id: area
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton
		hoverEnabled: true

		onClicked: {
			if (userDetails && !userDetails.running) {
				if (userDetails.opened)
					userDetails.close()
				else
					userDetails.open()
			}
			control.clicked()
		}
	}
}
