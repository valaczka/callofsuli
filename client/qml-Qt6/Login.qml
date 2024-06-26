import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

QScrollable
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	contentCentered: !registrationMode

	property bool registrationMode: false

	Connections {
		target: Client.server ? Client.server.user : null

		function onLoginStateChanged() {
			if (Client.server.user.loginState == User.LoggedIn) {
				registrationMode = false
				_password.clear()
			}
		}
	}


	Connections {
		target: Client

		function onLoadRequestRegistration(oauth, code) {
			registrationMode = true

			registration.code = code
			if (oauth === "google") {
				registration.googleChecked = true
				registration.button.clicked()
			} else
				registration.plainChecked = true

		}
	}


	Qaterial.GroupBox {
		id: _loginBox
		visible: Client.server && Client.server.user.loginState == User.LoggedOut && !registrationMode

		anchors.horizontalCenter: parent.horizontalCenter
		width: Math.min(parent.width, implicitWidth)

		implicitWidth: 400

		color: Client.Utils.colorSetAlpha("black", 0.7)


		Column {
			width: parent.width-2*spacing
			anchors.horizontalCenter: parent.horizontalCenter

			spacing: 15
			topPadding: spacing
			bottomPadding: spacing

			QButton {
				id: _google
				visible: Client.server && Client.server.config.oauthProviders !== undefined && Client.server.config.oauthProviders.includes("google")
						 && !_loginFields.visible
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Bejelentkezés Google fiókkal")
				icon.source: Qaterial.Icons.google
				onClicked: Client.loginOAuth2("google")
			}

			QButton {
				id: _microsoft
				visible: Client.server && Client.server.config.oauthProviders !== undefined &&
						 Client.server.config.oauthProviders.includes("microsoft") && !_loginFields.visible
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Bejelentkezés Microsoft fiókkal")
				icon.source: Qaterial.Icons.microsoft
				onClicked: Client.loginOAuth2("microsoft")
			}


			QButton {
				visible: !_loginFields.visible
				flat: true
				outlined: false

				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Bejelentkezés jelszóval")

				onClicked: {
					_loginFields.visible = true
					_user.forceActiveFocus()
				}
			}

			QFormColumn {
				id: _loginFields
				spacing: 5

				visible: false

				QFormTextField {
					id: _user
					width: parent.width
					title: qsTr("Felhasználónév")
					onAccepted: _password.forceActiveFocus()
					text: Client.server && Client.server.user && Client.server.user.oauth == "" ? Client.server.user.username : ""
					inputMethodHints: Qt.ImhNoPredictiveText
				}

				QFormTextField {
					id: _password
					width: parent.width
					title: qsTr("Jelszó")
					echoMode: TextInput.Password
					inputMethodHints: Qt.ImhSensitiveData
					trailingContent: Qaterial.TextFieldButtonContainer
					{
						Qaterial.TextFieldPasswordButton {  }
					}
					onAccepted: _send.clicked()
				}

				QButton {
					id: _send
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Bejelentkezés")
					icon.source: Qaterial.Icons.accountCheck
					enabled: _user.length && _password.length
					onClicked: Client.loginPlain(_user.text, _password.text)
				}
			}


			Qaterial.HorizontalLineSeparator {
				visible: _registration.visible
				anchors.horizontalCenter: parent.horizontalCenter
				width: parent.width*0.75
			}

			QButton {
				id: _registration
				visible: Client.server && Client.server.config.registrationEnabled !== undefined && Client.server.config.registrationEnabled
						 && !_loginFields.visible
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Regisztráció")
				icon.source: Qaterial.Icons.loginVariant
				onClicked: registrationMode = true // Client.stackPushPage("Registration.qml")
			}


		}
	}


	Column {
		visible: Client.server && Client.server.user.loginState != User.LoggedOut && !registrationMode

		anchors.horizontalCenter: parent.horizontalCenter
		spacing: 20

		Row {
			spacing: 20

			Qaterial.BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
				height: txt.height
				width: txt.height
				visible: Client.server && Client.server.user.loginState == User.LoggingIn
			}

			Qaterial.IconLabelWithCaption {
				id: txt
				icon.source: Client.server && Client.server.user.loginState == User.LoggedIn ? Qaterial.Icons.account : ""
				anchors.verticalCenter: parent.verticalCenter
				text: Client.server && Client.server.user.loginState == User.LoggedIn ? qsTr("Bejelentkeztél") : qsTr("Bejelentkezés...")
				caption: Client.server ? Client.server.user.username : ""
			}
		}

		Qaterial.RaisedButton {
			visible: Client.server && Client.server.user.loginState == User.LoggingIn
			anchors.horizontalCenter: parent.horizontalCenter
			backgroundColor: Qaterial.Colors.red600
			foregroundColor: Qaterial.Colors.white
			text: qsTr("Megszakítás")
			icon.source: Qaterial.Icons.close
			onClicked: Client.httpConnection.abort()
		}
	}


	Registration {
		id: registration
		visible: registrationMode
		anchors.horizontalCenter: parent.horizontalCenter
	}
}
