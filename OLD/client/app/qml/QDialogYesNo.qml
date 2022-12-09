import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS
import "."


QDialogPanel {
	id: item

	title: qsTr("Kérdés")
	property alias text: labelText.text
	property alias image: bigImage.icon

	maximumHeight: 250
	maximumWidth: 650

	icon: "qrc:/internal/icon/message-question.svg"
	titleColor: CosStyle.colorPrimaryDarker


	Row {
		id: rw

		anchors.centerIn: parent

		QFontImage {
			id: bigImage
			visible: icon != ""
			color: CosStyle.colorPrimaryLighter
			size: Math.min(item.panel.height*0.5, CosStyle.pixelSize*4.5)
			width: visible ? size+CosStyle.pixelSize*2 : 0
			icon: ""

			anchors.verticalCenter: parent.verticalCenter
		}

		QLabel {
			id: labelText
			color: CosStyle.colorPrimaryLighter

			width: Math.min(rw.parent.width-bigImage.width, implicitWidth)
			anchors.verticalCenter: parent.verticalCenter

			font.weight: Font.Medium

			wrapMode: Text.WordWrap
			horizontalAlignment: Qt.AlignHCenter
			verticalAlignment: Qt.AlignVCenter
		}
	}

	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Nem")
			icon.source: "qrc:/internal/icon/close-circle.svg"
			themeColors: CosStyle.buttonThemeRed

			onClicked: {
				dlgClose()
			}
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("Igen")
			icon.source: "qrc:/internal/icon/check-bold.svg"
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				acceptedData = true
				dlgClose()
			}
		}
	}

	function populated() {
		buttonYes.forceActiveFocus()
	}
}
