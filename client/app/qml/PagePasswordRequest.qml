import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: qsTr("Elfelejtett jelszó")

	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: QSimpleContainer {
			id: panel

			title: qsTr("Új jelszó kérése")
			icon: CosStyle.iconRegistration
			maximumWidth: 600

			onPopulated: textEmail.forceActiveFocus()

			QGridLayout {
				id: grid

				watchModification: false

				QGridLabel { field: textEmail }

				QGridTextField {
					id: textEmail
					fieldName: qsTr("E-mail cím")

					onAccepted: buttonCode.press()

					validator: RegExpValidator { regExp: /^(.+@..+\...+)$/ }
				}

				QGridLabel { field: textCode }

				QGridTextField {
					id: textCode
					fieldName: qsTr("Aktivációs kód")
					validator: RegExpValidator { regExp: /.+/ }

					onAccepted: buttonSend.press()
				}

				QGridButton {
					id: buttonSend
					enabled: textEmail.acceptableInput && textCode.acceptableInput
					text: qsTr("Küldés")

					onClicked: {
						grid.enabled = false
						cosClient.passwordRequest(textEmail.text, textCode.text)
					}
				}

				QGridButton {
					id: buttonCode
					enabled: textEmail.acceptableInput && !textCode.acceptableInput
					text: qsTr("Aktivációs kód kérése")

					onClicked: {
						grid.enabled = false
						cosClient.passwordRequest(textEmail.text)
					}
				}
			}
		}

		Connections {
			target: cosClient

			function onResetPasswordEmailSent() {
				grid.enabled = true
			}

			function onResetPasswordFailed() {
				grid.enabled = true
			}

			function onResetPasswordSuccess() {
				cosClient.login(textEmail.text, "", "")
			}
		}
	}

	onPageActivated: textEmail.forceActiveFocus()

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}

