import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QPage {
	id: page

	mainToolBar.title: qsTr("Jelszó beállítása")


	panelComponents: [
		Component {
			QPagePanel {
				id: panel

				title: qsTr("Jelszó beállítása")
				maximumWidth: 600
				layoutFillWidth: true


				QGridLayout {
					id: grid

					anchors.fill: parent

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
	]


	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}

}
