import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 400

	property alias title: mainRow.title
	property alias text: labelText.text

	signal dlgClose()
	signal dlgAccept(var data)

	Item {
		id: dlgItem
		anchors.centerIn: parent

		width: Math.min(parent.width*0.9, 650)
		height: Math.min(parent.height*0.9, 300)



		BorderImage {
			id: bgRectMask
			source: "qrc:/img/border.svg"
			visible: false

			anchors.fill: bgRectData

			border.left: 15; border.top: 10
			border.right: 15; border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		Rectangle {
			id: bgRectData

			anchors.fill: rectBg
			visible: false

			color: JS.setColorAlpha(CosStyle.colorPrimaryDark, 0.2)
		}

		OpacityMask {
			id: rectBg
			source: bgRectData
			maskSource: bgRectMask

			anchors.top: parent.top
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.bottom: buttonRow.top
			anchors.bottomMargin: 30

			QDialogHeader {
				id: mainRow
				icon: "M\ue887"
			}

			DropShadow {
				anchors.fill: labelText
				horizontalOffset: 2
				verticalOffset: 2
				radius: 1
				samples: 3
				source: labelText
				visible: true
			}

			Label {
				id: labelText
				color: CosStyle.colorPrimaryLighter
				anchors.top: mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 30
				anchors.topMargin: 20
				wrapMode: Text.WordWrap
				horizontalAlignment: Qt.AlignHCenter
				verticalAlignment: Qt.AlignVCenter
			}
		}

		Row {
			id: buttonRow
			spacing: 10

			anchors.horizontalCenter: parent.horizontalCenter
			anchors.bottom: parent.bottom

			QButton {
				id: buttonNo
				anchors.verticalCenter: parent.verticalCenter
				label: qsTr("Nem")
				icon: "M\ue5cd"
				bgColor: CosStyle.colorErrorDarker
				borderColor: CosStyle.colorErrorDark

				onClicked: dlgClose()
			}

			QButton {
				id: buttonYes

				anchors.verticalCenter: parent.verticalCenter

				label: qsTr("Igen")
				icon: "M\ue5ca"

				bgColor: CosStyle.colorOKDarker
				borderColor: CosStyle.colorOKDark

				onClicked: dlgAccept(true)
			}
		}

	}

	function populated() {
		buttonYes.forceActiveFocus()
	}
}
