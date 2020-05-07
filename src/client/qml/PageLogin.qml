import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	property bool isBusy: false

	header: QToolBar {
		id: toolbar

		title: cosClient.serverName

		backButton.visible: true
		backButton.onClicked: mainStack.back()

		Row {
			rightPadding: 5
			Layout.fillHeight: true
			QToolBusyIndicator { running: page.isBusy }
		}
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

				onClicked: JS.createPage("PasswordRequest", {}, page)
			}
		}
	}


	Connections {
		target: cosClient

		onAuthInvalid: page.isBusy = false
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
