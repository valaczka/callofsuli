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

		title: qsTr("Jelszó beállítása")

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

	Connections {
		target: cosClient
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		textUser.forceActiveFocus()
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
