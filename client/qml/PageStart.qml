import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	//closeQuestion: StackView.index < 3 ? "Nem tudod, miért nem szeretnéd bezárni?" : ""
	//closeDisabled: (StackView.index == 4) ? "Nem lehet bezárni!" : ""

	/*stackPopFunction: function() {
		console.debug("STACK POP")
		return false
	}*/

	title: "Call of Suli"

	appBar.backButtonVisible: false
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionDemo }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionAbout }
			QMenuItem { action: actionExit }
		}

	}

	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		icon.source: "qrc:/internal/img/callofsuli_square.svg"
		onTriggered: {
			onClicked: Client.stackPushPage("PageAbout.qml", {})
		}
	}

	Action {
		id: actionExit
		icon.source: Qaterial.Icons.applicationExport
		text: qsTr("Kilépés")
		onTriggered: Client.closeWindow()
	}


	Action {
		id: actionDemo
		text: qsTr("Demo")
		icon.source: Qaterial.Icons.presentationPlay
		shortcut: "F2"
		onTriggered: {
			onClicked: Client.loadDemoMap()
		}
	}


	Row {
		id: row1

		Qaterial.RaisedButton {
			text: "Start Game"
			icon.source: Qaterial.Icons.gamepad
			onClicked: Client.loadGame()
			foregroundColor: Qaterial.Colors.black
		}

		Qaterial.RaisedButton {
			text: "Db"
			highlighted: false

		}


		Qaterial.RaisedButton {
			text: "Connect"
			highlighted: Client.webSocket.state == WebSocket.Connected

			onClicked: Client.testConnect()

		}

		Qaterial.RaisedButton {
			text: "Hello"
			highlighted: false
			onClicked: Client.testHello()
		}

		Qaterial.RaisedButton {
			text: "Request"
			highlighted: false
			onClicked: Client.testRequest()
		}


		Qaterial.RaisedButton {
			text: "Google"
			highlighted: false
			onClicked: Client.testText()
		}

		Qaterial.RaisedButton {
			text: "Close"
			highlighted: false
			onClicked: Client.testClose()
		}


	}

	Row {
		id: row2
		anchors.left: parent.left
		anchors.top: row1.bottom

		spacing: 10

		Qaterial.TextField {
			id: fUser
			title: "Username"
			helperText: "Felhasználónév"
			width: 300

			trailingContent:   Qaterial.TextFieldClearButton {textField: fUser}
		}

		Qaterial.FilledTextField {
			id: fPassword
			title: "Password"
			helperText: "Jelszó"
			width: 300

			echoMode: TextInput.Password
			inputMethodHints: Qt.ImhSensitiveData
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				Qaterial.TextFieldPasswordButton {textField: fPassword} // TextFieldCopyButton
				Qaterial.TextFieldClearButton {textField: fPassword} // TextFieldClearButton
			}
		}

		Qaterial.RaisedButton {
			text: "Login"
			highlighted: false
			onClicked: Client.testText(fUser.text, fPassword.text)
		}
	}

	Qaterial.TextField {
		id: fToken
		anchors.top: row2.bottom
		anchors.left: parent.left
		anchors.right: btnSend.left
	}

	Qaterial.RaisedButton {
		id: btnSend
		anchors.verticalCenter: fToken.verticalCenter
		anchors.right: parent.right
		text: "SEND"
		highlighted: false
		onClicked: Client.testToken(fToken.text)
	}



	//StackView.onActivated: Client.loadGame()
}
