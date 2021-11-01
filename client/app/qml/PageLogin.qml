import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: cosClient.serverName
	defaultSubTitle: qsTr("Bejelentkezés")


	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: QSimpleContainer {
			id: panel

			title: qsTr("Bejelentkezés")
			icon: CosStyle.iconLogin
			maximumWidth: 600

			QGridLayoutFlickable {
				id: grid

				//enabled: !page.isBusy
				watchModification: false

				onAccepted: buttonLogin.press()

				QGridLabel { field: textUser }

				QGridTextField {
					id: textUser
					fieldName: qsTr("Felhasználónév")

					validator: RegExpValidator { regExp: /.+/ }
				}

				QGridLabel { field: textPassword }

				QGridTextField {
					id: textPassword
					fieldName: qsTr("Jelszó")
					echoMode: TextInput.Password

				}

				QGridButton {
					id: buttonLogin
					text: qsTr("Bejelentkezés")
					icon.source: CosStyle.iconLogin
					enabled: textUser.acceptableInput &&
							 textPassword.acceptableInput

					onClicked: {
						cosClient.login(textUser.text, "", textPassword.text)
					}
				}

				QGridButton {
					id: buttonForgot
					text: qsTr("Elfelejtettem a jelszavam")

					enabled: cosClient.passwordResetEnabled
					visible: cosClient.passwordResetEnabled

					onClicked: JS.createPage("PasswordRequest", {})
				}
			}

			Connections {
				target: control
				function onPageActivated() {
					textUser.forceActiveFocus()
				}
			}

			onPopulated: textUser.forceActiveFocus()

		}


	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}

