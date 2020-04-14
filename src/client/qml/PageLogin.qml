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

				validator: RegExpValidator { regExp: /[A-Za-z0-9_-.]+/ }
			}

			QGridLabel { field: textPassword }

			QGridTextField {
				id: textPassword
				fieldName: qsTr("Jelszó")
				echoMode: TextInput.Password

				validator: RegExpValidator { regExp: /.+/ }
			}

			QGridButton {
				id: buttonLogin
				label: qsTr("Bejelentkezés")
				disabled: !textUser.acceptableInput ||
						  !textPassword.acceptableInput

				onClicked: cosClient.login(textUser.text, "", textPassword.text)

			}
		}
	}

	Connections {
		target: cosClient
		onUserRolesChanged: if (!(cosClient.userRoles & Client.RoleGuest))
								mainStack.back()
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
