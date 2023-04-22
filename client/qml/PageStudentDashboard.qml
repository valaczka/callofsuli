import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (_closeEnabled || (Client.server && Client.server.user.loginState != User.LoggedIn))
			return true

		if (tabBar.currentIndex > 0) {
			tabBar.decrementCurrentIndex()
			return false
		}

		JS.questionDialog(
					{
						onAccepted: function()
						{
							_closeEnabled = true
							if (Client.server && Client.server.user.loginState == User.LoggedIn)
								Client.webSocket.close()
							else
								Client.stackPop(control)
						},
						text: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?"),
						title: qsTr("Kilépés"),
						iconSource: Qaterial.Icons.closeCircle
					})


		return false
	}

	header: null

	property StudentGroupList studentGroupList: Client.cache("studentGroupList")
	property CampaignList studentCampaignList: Client.cache("studentCampaignList")
	property StudentMapHandler studentMapHandler: StudentMapHandler {  }

	property bool _closeEnabled: false

	Qaterial.StackView
	{
		id: stackView
		anchors.fill: parent
	}

	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.webSocket.pending
	}

	footer: QTabBar
	{
		id: tabBar

		onCurrentIndexChanged: {
			if (!stackView || currentIndex == -1)
				return

			stackView.replace(model.get(currentIndex).cmp)
		}

		Component.onCompleted: {
			model.append({ text: qsTr("Áttekintés"), source: Qaterial.Icons.speedometer, cmp: cmpDashboard })
			model.append({ text: qsTr("Csoportjaim"), source: Qaterial.Icons.accountMultiple, cmp: cmpGroupList })
			model.append({ text: qsTr("Rangsor"), source: Qaterial.Icons.podium, cmp: cmpScoreList })
			model.append({ text: qsTr("Profil"), source: Qaterial.Icons.account, cmp: cmpRect })
		}
	}


	Component {
		id: cmpDashboard
		StudentDashboard {
			studentMapHandler: control.studentMapHandler
		}
	}

	Component {
		id: cmpGroupList
		StudentGroups {  }
	}

	Component {
		id: cmpScoreList
		QItemGradient {
			id: _pageScoreList
			ScoreList {
				id: _scoreList
				anchors.fill: parent
				paddingTop: _pageScoreList.paddingTop
			}

			appBar.rightComponent: Qaterial.AppBarButton {
				icon.source: Qaterial.Icons.medal
				ToolTip.text: qsTr("Ranglista")

				onClicked: Client.stackPushPage("Ranks.qml")
			}

			StackView.onActivated: _scoreList.reload()
		}
	}

	Component {
		id: cmpRect

		Rectangle {
			color: "green"

			QButton {
				anchors.centerIn: parent
				text: "Logout"
				onClicked: Client.logout()
			}
		}
	}

	StackView.onActivated: {
		Client.reloadUser()
		Client.reloadCache("studentGroupList")
		Client.reloadCache("studentCampaignList")
		studentMapHandler.reload()
	}
}
