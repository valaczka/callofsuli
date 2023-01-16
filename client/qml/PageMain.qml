import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?")

	onPageClose: function() {
		Client.webSocket.close()
	}

	title: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: UserButton { }

	/*appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: view.visible
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionAdd }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionDemo }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionAbout }
			QMenuItem { action: actionExit }
		}

	}*/


	Row {
		Qaterial.LabelBody1 {
			text: "Bent vagyunk"
		}

		QButton {
			text: "Close"
			onClicked: Client.sendRequest(WebSocketMessage.ClassGeneral,{func: "rankList"})
		}
	}
}
