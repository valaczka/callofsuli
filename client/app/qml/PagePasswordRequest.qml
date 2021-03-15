import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPage {
	id: page

	title: qsTr("Elfelejtett jelszó")

	onlyPanel: QPagePanel {
		id: panel

		title: page.title
		maximumWidth: 600

		onPopulated: textEmail.forceActiveFocus()

		Connections {
			target: page
			onPageActivated: textEmail.forceActiveFocus()
		}


		QGridLayout {
			id: grid

			anchors.fill: parent

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

				onClicked: cosClient.passwordRequest(textEmail.text, textCode.text)
			}

			QGridButton {
				id: buttonCode
				enabled: textEmail.acceptableInput && !textCode.acceptableInput
				text: qsTr("Aktivációs kód kérése")

				onClicked: cosClient.passwordRequest(textEmail.text)
			}
		}
	}

	Connections {
		target: cosClient

		onAuthPasswordResetSuccess: cosClient.login(textEmail.text, "", "")
	}



	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}

}


