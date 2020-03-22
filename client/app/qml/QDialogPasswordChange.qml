import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS
import "."

QDialogPanel {
	id: item

	maximumHeight: 400
	maximumWidth: 750

	property alias username: textUser.text
	property alias password: textPassword.text
	property alias oldPassword: textPasswordOld.text

	title: qsTr("Jelszó megváltoztatása")
	icon: CosStyle.iconDialogQuestion

	titleColor: CosStyle.colorPrimary

	panel.anchors.topMargin: 10

	acceptedData: -1

	QGridLayoutFlickable {
		id: grid

		implicitWidth: item.panel.width-10

		onAccepted: buttonYes.press()

		QGridLabel { field: textUser }

		QGridTextField {
			id: textUser
			fieldName: qsTr("Felhasználónév")

			readOnly: true
		}

		QGridLabel { field: textPasswordOld }

		QGridTextField {
			id: textPasswordOld
			fieldName: qsTr("Régi jelszó")
			echoMode: TextInput.Password
			lineVisible: true
		}

		QGridLabel { field: textPassword }

		QGridTextField {
			id: textPassword
			fieldName: qsTr("Új jelszó")
			echoMode: TextInput.Password

			validator: RegExpValidator { regExp: /.+/ }

			lineVisible: true
		}

		QGridLabel { field: textPassword2 }

		QGridTextField {
			id: textPassword2
			fieldName: qsTr("Új jelszó ismét")
			echoMode: TextInput.Password

			validator: RegExpValidator { regExp: /.+/ }

			lineVisible: true
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
			themeColors: CosStyle.buttonThemeRed

			onClicked: dlgClose()
		}

		QButton {
			id: buttonYes

			anchors.verticalCenter: parent.verticalCenter

			text: qsTr("OK")
			icon.source: CosStyle.iconOK
			themeColors: CosStyle.buttonThemeGreen

			enabled: textPassword.acceptableInput &&
					 textPassword2.acceptableInput &&
					 textPassword.text === textPassword2.text

			onClicked: {
				acceptedData = 1
				dlgClose()
			}
		}
	}


	function populated() {
		textPasswordOld.forceActiveFocus()
	}

}
