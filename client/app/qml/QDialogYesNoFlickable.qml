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
	property alias textFormat: labelText.textFormat

	//maximumHeight: 250
	maximumWidth: 700

	property real panelPaddingLeft: 5
	property real panelPaddingRight: 5

	icon: CosStyle.iconDialogQuestion
	titleColor: CosStyle.colorPrimaryDarker
	property alias textColor: labelText.color


	Flickable {
		id: flick

		width: parent.width
		height: Math.min(parent.height-10, flick.contentHeight)
		anchors.verticalCenter: parent.verticalCenter

		clip: true

		contentWidth: labelText.width
		contentHeight: labelText.height

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick

		QLabel {
			id: labelText

			width: flick.width-item.panelPaddingLeft-item.panelPaddingRight
			x: item.panelPaddingLeft

			wrapMode: Text.Wrap
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
		buttonNo.forceActiveFocus()
	}
}
