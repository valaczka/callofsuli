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

	maximumHeight: 250
	maximumWidth: 650

	icon: CosStyle.iconDialogQuestion
	titleColor: CosStyle.colorPrimaryDarker


	QLabel {
		id: labelText
		color: CosStyle.colorPrimaryLighter

		width: parent.width
		anchors.centerIn: parent

		font.weight: Font.Medium

		wrapMode: Text.WordWrap
		horizontalAlignment: Qt.AlignHCenter
		verticalAlignment: Qt.AlignVCenter
	}

	buttons: Row {
		id: buttonRow
		spacing: 10

		anchors.horizontalCenter: parent.horizontalCenter

		QButton {
			id: buttonNo
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Nem")
			icon.source: CosStyle.iconCancel
			themeColors: CosStyle.buttonThemeRed

			onClicked: {
				dlgClose()
			}
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("Igen")
			icon.source: CosStyle.iconOK
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
