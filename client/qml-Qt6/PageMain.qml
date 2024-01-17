import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	onPageClose: function() {
		Client.httpConnection.close()
	}

	stackPopFunction: function() {
		if (stackView.depth > 1) {
			stackView.pop()
			return false
		}

		if (_login.registrationMode) {
			_login.registrationMode = false
			return false
		}

		return true
	}

	title: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: stackView.depth == 2 ? _cmpRank : _cmpUser
	appBar.rightPadding: Qaterial.Style.horizontalPadding

	Component {
		id: _cmpUser
		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.podium
			ToolTip.text: qsTr("Ranglista")
			onClicked: stackView.push(_cmpScoreList)

		}
	}

	Component {
		id: _cmpRank

		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.medal
			ToolTip.text: qsTr("Rangok")
			onClicked: Client.stackPushPage("Ranks.qml")

		}
	}

	Qaterial.StackView
	{
		id: stackView
		anchors.fill: parent

		initialItem: Login {
			id: _login
		}
	}

	Component {
		id: _cmpScoreList

		ScoreList {
			StackView.onActivated: reload()
		}
	}

	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.httpConnection.pending
	}

}
