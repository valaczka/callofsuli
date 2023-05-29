import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
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

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	readonly property string _campaignName: campaign ? campaign.readableName : ""

	title: campaign ? _campaignName : qsTr("Új hadjárat")
	subtitle: group ? group.fullName : ""


	appBar.backButtonVisible: true

	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: swipeView.currentIndex == 0 ? menuDetails.open() : menuResult.open()

		QMenu {
			id: menuDetails

			QMenuItem { action: _actionRemove }
		}

		QMenu {
			id: menuResult

			QMenuItem { action: _actionResultReload }
		}
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherCampaignDetails {
			group: control.group
			campaign: control.campaign
			mapHandler: control.mapHandler
		}

		TeacherCampaignResult {
			id: _result
			group: control.group
			campaign: control.campaign
			mapHandler: control.mapHandler
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Hadjárat"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.trophyBroken, color: "pink" })
		}
	}


	Action {
		id: _actionResultReload
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: _result.resultModel.reloadContent()
	}

	Action {
		id: _actionRemove

		enabled: campaign && campaign.state < Campaign.Running
		text: qsTr("Hadjárat törlése")
		icon.source: Qaterial.Icons.trashCan
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(WebSocket.ApiTeacher, "campaign/%1/delete".arg(campaign.campaignid))
								 .done(function(r){
									 group.reload()
									 Client.stackPop(control)
								 })
								 .fail(JS.failMessage("Törlés sikertelen"))
							 },
							 text: qsTr("Biztosan törlöd a hadjáratot?"),
							 title: _campaignName,
							 iconSource: Qaterial.Icons.closeCircle
						 })
	}

}
