import QtQuick 2.14
import QtQuick.Controls 2.14
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS
import "."

Item {
	id: item

	implicitHeight: 300
	implicitWidth: 400

	property alias title: mainRow.title
	property alias newField: tfInput
	property alias list: list
	property alias model: list.model

	signal dlgClose()
	signal dlgAccept()

	Item {
		id: dlgItem
		anchors.centerIn: parent

		width: Math.min(parent.width*0.9, 650)
		height: parent.height*0.9



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
			anchors.bottomMargin: 10

			QDialogHeader {
				id: mainRow
				icon: "M\ue887"
			}




			QTextField {
				id: tfInput

				anchors.top: mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right
			}



			QListItemDelegate {
				id: list
				anchors.top: tfInput.visible ? tfInput.bottom : mainRow.bottom
				anchors.left: parent.left
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 30
				anchors.topMargin: 0
				anchors.bottomMargin: mainRow.padding
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
				label: qsTr("Mégsem")
				icon: "M\ue5cd"
				bgColor: CosStyle.colorErrorDarker
				borderColor: CosStyle.colorErrorDark

				onClicked: dlgClose()
			}

			QButton {
				id: buttonYes

				anchors.verticalCenter: parent.verticalCenter

				label: qsTr("OK")
				icon: "M\ue5ca"

				bgColor: CosStyle.colorOKDarker
				borderColor: CosStyle.colorOKDark

				onClicked: dlgAccept()
			}
		}

	}

	function populated() {
		tfInput.forceActiveFocus()
	}

}
