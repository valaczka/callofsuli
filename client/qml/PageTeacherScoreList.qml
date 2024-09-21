import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server && Client.server.user ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Row {
		Qaterial.AppBarButton {
			anchors.verticalCenter: parent.verticalCenter
			icon.source: Qaterial.Icons.refresh
			ToolTip.text: qsTr("Frissítés")

			onClicked: _scoreList.reload()
		}
		Qaterial.AppBarButton {
			anchors.verticalCenter: parent.verticalCenter
			icon.source: Qaterial.Icons.medal
			ToolTip.text: qsTr("Ranglista")

			onClicked: Client.stackPushPage("Ranks.qml")
		}
	}


	ScoreList {
		id: _scoreList
		anchors.fill: parent
	}


	StackView.onActivated: _scoreList.reload()
}
