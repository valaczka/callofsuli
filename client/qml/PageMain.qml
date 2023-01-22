import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

//	closeQuestion: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?")

	onPageClose: function() {
		Client.webSocket.close()
	}

	stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
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
	appBar.rightComponent: swipeView.currentIndex == 1 ? _cmpRank : _cmpUser
	appBar.rightPadding: Qaterial.Style.horizontalPadding

	Component {
		id: _cmpUser
		UserButton { }
	}

	Component {
		id: _cmpRank

		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.medal
			ToolTip.text: qsTr("Ranglista")
			ToolTip.visible: hovered || pressed
			ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

			onClicked: Client.stackPushPage("Ranks.qml")

		}
	}

	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		Login {
			id: _login
			//height: swipeView.height
			//width: swipeView.width
		}

		ScoreList {
			//height: swipeView.height
			//width: swipeView.width
		}
	}

	footer: Qaterial.TabBar
	{
		id: tabBar
		width: parent.width
		currentIndex: swipeView.currentIndex

		Repeater
		{
			id: repeater

			model: ListModel
			{
				id: tabBarModel
			}

			delegate: Qaterial.TabButton
			{
				width: tabBar.width / model.count
				implicitWidth: width
				text: model.text ? model.text : ""
				icon.source: model.source ? model.source : ""
				spacing: 4
				display: (index === tabBar.currentIndex) ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
				font: Qaterial.Style.textTheme.overline
			}
		}

		Component.onCompleted: {
			tabBarModel.append({ text: qsTr("Bejelentkezés"), source: Qaterial.Icons.account })
			tabBarModel.append({ text: qsTr("Rangsor"), source: Qaterial.Icons.trophyBroken })
			//tabBar.setCurrentIndex(1)
		}

	}
}
