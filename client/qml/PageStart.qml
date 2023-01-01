import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
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

	}



	//StackView.onActivated: Client.loadGame()
}
