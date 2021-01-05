import QtQuick 2.12
import QtQuick.Controls 2.12
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	property alias text: labelText.text

	property alias value: tfInput.text
	property alias textField: tfInput

	maximumHeight: 300
	maximumWidth: 750


	icon: CosStyle.iconDialogQuestion

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
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeDelete

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeApply

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
