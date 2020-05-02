import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	title: cosClient.serverName

	header: QToolBar {
		id: toolbar

		title: qsTr("Elfelejtett jelszó")

		backButton.visible: true
		backButton.onClicked: mainStack.back()
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPagePanel {
		id: p

		anchors.fill: parent
		maximumWidth: 600

		blurSource: bgImage

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


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		textEmail.forceActiveFocus()
	}



	function windowClose() {
		return true
	}

	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		return false
	}
}
