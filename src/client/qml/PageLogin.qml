import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QPage {
	id: control

	requiredPanelWidth: 900

	mainToolBar.title: cosClient.serverName

	panelComponents: [
		Component {
			QPagePanel {
				id: panel

				title: qsTr("Bejelentkezés")
				maximumWidth: 600
				panelVisible: true
				layoutFillWidth: true

				onPanelActivated: textUser.forceActiveFocus()

				QGridLayout {
					id: grid

					anchors.fill: parent

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
	]



	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}
}

