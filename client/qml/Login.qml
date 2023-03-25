import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

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
			if (Client.server.user.loginState == User.LoggedIn)
				registrationMode = false
		}
	}


	Connections {
		target: Client

		function onLoadRequestRegistration(oauth, code) {
			console.debug("LOAD", oauth, code)

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

			QFormColumn {
				spacing: 5

				QFormTextField {
					id: _user
					width: parent.width
					title: qsTr("Felhasználónév")
					onAccepted: _password.forceActiveFocus()
					text: Client.server && Client.server.user && Client.server.user.oauth == "" ? Client.server.user.username : ""
				}

				QFormTextField {
					id: _password
					width: parent.width
					title: qsTr("Jelszó")
					echoMode: TextInput.Password
					inputMethodHints: Qt.ImhSensitiveData
					trailingContent: Qaterial.TextFieldPasswordButton { textField: _password }
					onAccepted: _send.clicked()
				}

				QButton {
					id: _send
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Bejelentkezés")
					icon.source: Qaterial.Icons.account
					enabled: _user.length && _password.length
					onClicked: Client.loginPlain(_user.text, _password.text)
				}
			}


			Qaterial.HorizontalLineSeparator {
				visible: _google.visible
				anchors.horizontalCenter: parent.horizontalCenter
				width: parent.width*0.75
			}

			QButton {
				id: _google
				visible: Client.server && Client.server.config.oauthGoogle !== undefined && Client.server.config.oauthGoogle
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Bejelentkezés Google fiókkal")
				icon.source: Qaterial.Icons.google
				onClicked: Client.loginGoogle()
			}

			Qaterial.HorizontalLineSeparator {
				visible: _registration.visible
				anchors.horizontalCenter: parent.horizontalCenter
				width: parent.width*0.75
			}

			QButton {
				id: _registration
				visible: Client.server && Client.server.config.registrationEnabled !== undefined && Client.server.config.registrationEnabled
				anchors.horizontalCenter: parent.horizontalCenter
				text: qsTr("Regisztráció")
				icon.source: Qaterial.Icons.login
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
			onClicked: Client.webSocket.abort()
		}
	}


	Registration {
		id: registration
		visible: registrationMode
		anchors.horizontalCenter: parent.horizontalCenter
	}
}
