import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "JScript.js" as JS

QPage {
	id: control

	signal userLoaded()

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

	property StudentMapHandler studentMapHandler: StudentMapHandler {  }

	property bool _closeEnabled: false

	Loader {
		id: _loader
		asynchronous: true
		anchors.fill: parent
		sourceComponent: Qaterial.StackView {  }
		onLoaded: {
			if (tabBar.currentIndex > -1)
				item.replace(tabBar.model.get(tabBar.currentIndex).cmp)
		}
	}


	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.webSocket.pending || _loader.status == Loader.Loading
	}

	footer: QTabBar
	{
		id: tabBar

		onCurrentIndexChanged: {
			if (!_loader.item || currentIndex < 0)
				return

			_loader.item.replace(model.get(currentIndex).cmp)
		}

		Component.onCompleted: {
			model.append({ text: qsTr("Áttekintés"), source: Qaterial.Icons.speedometer, cmp: cmpDashboard })
			model.append({ text: qsTr("Csoportjaim"), source: Qaterial.Icons.accountGroup, cmp: cmpGroupList })
			model.append({ text: qsTr("Rangsor"), source: Qaterial.Icons.podium, cmp: cmpScoreList })
			model.append({ text: qsTr("Profil"), source: Qaterial.Icons.account, cmp: cmpProfil })
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
		StudentGroups {
			studentMapHandler: control.studentMapHandler
		}
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

			StackView.onActivated: _scoreList.reload()
		}
	}

	Component {
		id: cmpProfil

		StudentProfile {
			user: Client.server.user
		}

	}

	StackView.onActivated: {
		Client.reloadUser()
		Client.reloadCache("studentGroupList")
		Client.reloadCache("studentCampaignList")
		studentMapHandler.reload()
	}
}

