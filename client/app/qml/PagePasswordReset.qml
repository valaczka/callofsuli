import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Új jelszó beállítása")

	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: QSimpleContainer {
			id: panel

			title: qsTr("Új jelszó")
			icon: CosStyle.iconRegistration
			maximumWidth: 600

			onPopulated: textUser.forceActiveFocus()


			QGridLayoutFlickable {
				id: grid

				watchModification: false

				onAccepted: buttonLogin.press()

				QGridLabel { field: textUser }

				QGridTextField {
					id: textUser
					fieldName: qsTr("Felhasználónév")

					text: cosClient.userName

					readOnly: true
				}

				QGridLabel { field: textPassword }

				QGridTextField {
					id: textPassword
					fieldName: qsTr("Új jelszó")
					echoMode: TextInput.Password

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textPassword2 }

				QGridTextField {
					id: textPassword2
					fieldName: qsTr("Új jelszó ismét")
					echoMode: TextInput.Password

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridButton {
					id: buttonLogin
					text: qsTr("Bejelentkezés")
					enabled: textUser.acceptableInput &&
							 textPassword.acceptableInput &&
							 textPassword2.acceptableInput &&
							 textPassword.text === textPassword2.text

					onClicked: cosClient.login(textUser.text, "", textPassword.text, true)
				}
			}
		}
	}

	onPageActivated: textUser.forceActiveFocus()

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}
