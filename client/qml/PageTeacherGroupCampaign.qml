import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}

		return true
	}

	title: group ? group.fullName : qsTr("Csoport")
	subtitle: Client.server ? Client.server.serverName : ""

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: swipeView.currentIndex == 0 ? menuCampaign.open() : menuResult.open()

		QMenu {
			id: menuCampaign

			QMenuItem { action: _campaignList.actionCampaignAdd }
		}

		QMenu {
			id: menuResult

			QMenuItem { action: _actionResultReload }
			QMenuItem { action: _result.actionUserEdit }
		}
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherGroupCampaignList {
			id: _campaignList
			group: control.group
			mapHandler: control.mapHandler
		}

		TeacherGroupCampaignResult {
			id: _result
			group: control.group
			mapHandler: control.mapHandler
		}

		TeacherGroupLog {
			id: _log
			group: control.group
			mapHandler: control.mapHandler
		}
	}

	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.httpConnection.pending
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Kihívások"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Résztvevők"), source: Qaterial.Icons.accountSupervisor, color: "green" })
			model.append({ text: qsTr("Log"), source: Qaterial.Icons.listBoxOutline, color: "yellow" })

			_tour.loadFromTabBar(tabBar, [1,2], 0)
		}
	}

	Action {
		id: _actionResultReload
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: _result.resultModel.reloadContent()
	}


	Component.onCompleted: {
		_tour.loadFromTabBar(tabBar, [1,2], 0)

	}

	StackView.onActivated: {
		_tour.start()
	}


	SpotlightCoachTour {
		id: _tour

		page: "teachercampaign"

		basePage: control

		list: [
			{ target: null, title: qsTr("Résztvevők"), text: qsTr("Itt tudod megtekinteni a tanulók elért eredményeit")},
			{ target: null, title: qsTr("Log"), text: qsTr("Itt tudod megtekinteni időrendben a diákok konkrét tevékenységeit")},
		]
	}
}
