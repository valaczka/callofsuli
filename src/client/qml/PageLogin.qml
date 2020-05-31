import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPage {
	id: page

	requiredPanelWidth: 900

	mainToolBar.title: cosClient.serverName

	mainToolBarComponent: QToolBusyIndicator { running: page.isBusy }

	property bool isBusy: false

	onlyPanel: QPagePanel {
		id: panel

		title: qsTr("Bejelentkezés")
		maximumWidth: 600

		onPanelActivated: textUser.forceActiveFocus()

		QGridLayout {
			id: grid

			anchors.fill: parent

			enabled: !page.isBusy
			watchModification: false

			onAccepted: buttonLogin.press()

			QGridLabel { field: textUser }

			QGridTextField {
				id: textUser
				fieldName: qsTr("Felhasználónév vagy email")

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
					page.isBusy = true
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
			target: page
			onPageActivated: textUser.forceActiveFocus()
		}

		onPopulated: textUser.forceActiveFocus()
	}



	Connections {
		target: cosClient

		onAuthInvalid: page.isBusy = false
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}

}

