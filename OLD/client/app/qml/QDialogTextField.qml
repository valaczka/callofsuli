import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias text: labelText.text

	property alias value: tfInput.text
	property alias textField: tfInput
	property alias inputMethodHints: tfInput.inputMethodHints

	maximumHeight: 300
	maximumWidth: 750


	icon: "qrc:/internal/icon/message-text.svg"

	titleColor: CosStyle.colorPrimary

	Column {
		anchors.centerIn: parent
		width: parent.width-30

		QLabel {
			id: labelText
			color: CosStyle.colorPrimaryLighter

			width: parent.width
			visible: text.length
			wrapMode: Text.WordWrap
			verticalAlignment: Qt.AlignVCenter
			horizontalAlignment: Qt.AlignLeft
			font.weight: Font.Medium
		}

		QTextField {
			id: tfInput

			width: parent.width

			lineVisible: true

			onAccepted: {
				acceptedData = text
				dlgClose()
			}
		}
	}

	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Mégsem")
			icon.source: "qrc:/internal/icon/close-circle.svg"
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("OK")
			icon.source: "qrc:/internal/icon/check-bold.svg"
			themeColors: CosStyle.buttonThemeGreen

			onClicked: {
				acceptedData = tfInput.text
				dlgClose()
			}
		}
	}


	function populated() {
		tfInput.forceActiveFocus()
	}

}
