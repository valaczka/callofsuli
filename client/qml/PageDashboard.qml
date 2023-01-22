import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	closeQuestion: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?")

	onPageClose: function() {
		if (Client.server && Client.server.user.loginState == User.LoggedIn)
			Client.webSocket.close()
	}


	/*stackPopFunction: function() {
		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}

		if (_login.registrationMode) {
			_login.registrationMode = false
			return false
		}

		return true
	}*/

	title: Client.server ? Client.server.serverName : ""
	subtitle: Client.server ? Client.server.user.fullName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: _cmpUser //swipeView.currentIndex == 1 ? _cmpRank : _cmpUser
	appBar.rightPadding: Qaterial.Style.horizontalPadding

	Component {
		id: _cmpUser
		UserButton { }
	}

	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		Item {
			Qaterial.LabelBody1 {
				anchors.centerIn: parent
				text: "text"
			}

		}

		ScoreList {
			height: swipeView.height
			width: swipeView.width
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
