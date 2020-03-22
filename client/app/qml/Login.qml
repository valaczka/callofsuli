import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	property Servers servers: null

	title: qsTr("Bejelentkezés")
	icon: CosStyle.iconLogin

	QGridLayoutFlickable {
		id: grid

		watchModification: false

		onAccepted: buttonLogin.press()

		QTabHeader {
			tabContainer: control
			Layout.columnSpan: parent.columns
			Layout.fillWidth: true
			isPlaceholder: true
		}

		QGridButton {
			id: buttonGoogle
			text: qsTr("Google fiókkal")
			icon.source: "qrc:/internal/img/google.svg"

			enabled: servers && servers.googleOAuth2
			visible: servers && servers.googleOAuth2

			onClicked: {
				control.enabled = false
				servers.googleOAuth2.grant()
			}
		}

		QGridButton {
			id: buttonText
			text: qsTr("Jelszóval")
			icon.source: CosStyle.iconUserWhite

			onClicked: {
				buttonText.visible = false
				buttonGoogle.visible = false
				textUser.visible = true
				textUserLabel.visible = true
				textPassword.visible = true
				textPasswordLabel.visible = true
				buttonLogin.visible = true
				textUser.forceActiveFocus()
			}
		}

		QGridLabel {
			id: textUserLabel
			field: textUser
			visible: false
		}

		QGridTextField {
			id: textUser
			visible: false
			fieldName: qsTr("Felhasználónév")
			inputMethodHints: Qt.ImhEmailCharactersOnly

			validator: RegExpValidator { regExp: /.+/ }
		}

		QGridLabel {
			id: textPasswordLabel
			field: textPassword
			visible: false
		}

		QGridTextField {
			id: textPassword
			visible: false
			fieldName: qsTr("Jelszó")
			echoMode: TextInput.Password
			inputMethodHints: Qt.ImhSensitiveData

		}

		QGridButton {
			id: buttonLogin
			visible: false
			text: qsTr("Bejelentkezés")
			icon.source: CosStyle.iconLogin
			enabled: textUser.acceptableInput &&
					 textPassword.acceptableInput

			onClicked: {
				labelLogin.visible = true
				control.enabled = false
				cosClient.login(textUser.text, "", textPassword.text)
			}
		}

		/*QGridButton {
			id: buttonForgot
			text: qsTr("Elfelejtettem a jelszavam")

			enabled: cosClient.passwordResetEnabled
			visible: cosClient.passwordResetEnabled

			onClicked: JS.createPage("PasswordRequest", {})
		}*/



		QLabel {
			id: labelLogin

			Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
			Layout.columnSpan: parent.columns
			Layout.fillWidth: true
			Layout.topMargin: 20
			Layout.bottomMargin: 10

			text: qsTr("Bejelentkezés...")
			color: CosStyle.colorOKLighter
			horizontalAlignment: Text.AlignHCenter

			font.weight: Font.DemiBold
			font.pixelSize: CosStyle.pixelSize*0.9

			visible: false
		}
	}



	onPopulated: {
		if (tabPage && !tabPage.loginTried && servers) {
			tabPage.loginTried = true

			if (servers.serverTryLogin()) {
				labelLogin.visible = true
				control.enabled = false
			}

		} else {
			control.enabled = true
			//textUser.forceActiveFocus()
		}
	}



	Component {
		id: oauthContainer
		OAuthContainer {  }
	}


	Connections {
		target: cosClient

		function onAuthInvalid() {
			labelLogin.visible = false
			control.enabled = true
		}
	}

	Connections {
		target: servers && servers.googleOAuth2 ? servers.googleOAuth2 : null

		function onBrowserRequest(url) {
			if (Qt.platform.os == "android"  || Qt.platform.os === "ios" || Qt.platform.os === "linux") {
				tabPage.pushContent(oauthContainer, {url: url})
			} else {
				cosClient.openUrl(url)
			}
		}

		function onAuthenticated(token) {
			labelLogin.visible = false
			control.enabled = false
			cosClient.oauth2Login(token)
		}
	}

}

