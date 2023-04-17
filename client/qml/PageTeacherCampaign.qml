import QtQuick 2.12
import QtQuick.Controls 2.12
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

	readonly property string _campaignName: campaign ? campaign.readableName : ""

	title: campaign ? _campaignName : qsTr("Új hadjárat")
	subtitle: group ? group.fullName : ""


	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: campaign && campaign.state < Campaign.Running
		ToolTip.text: qsTr("Hadjárat törlése")
		icon.source: Qaterial.Icons.trashCan
		onClicked: JS.questionDialog(
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

	appBar.backButtonVisible: true
	/*appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: swipeView.currentIndex == 0 ? menuClass.open() : menuCampaign.open()

		QMenu {
			id: menuClass

			QMenuItem { action: actionGroupRename }
			QMenuItem { action: actionGroupRemove }
		}

		QMenu {
			id: menuCampaign

			QMenuItem { action: _campaignList.actionCampaignAdd }
		}
	}*/


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherCampaignDetails {
			group: control.group
			campaign: control.campaign
		}

		Rectangle {
			color: "blue"
		}

		/*TeacherGroupMemberList {
			group: control.group
		}

		TeacherGroupCampaignList {
			id: _campaignList
			group: control.group
		}*/
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Hadjárat"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			/*model.append({ text: qsTr("Hadjáratok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.trophyBroken, color: "pink" })*/
		}
	}

/*	Action {
		id: actionGroupRemove
		text: qsTr("Törlés")
		enabled: group
		icon.source: Qaterial.Icons.trashCan
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.send(WebSocket.ApiTeacher, "group/%1/delete".arg(group.groupid))
								.done(function(r){
									Client.reloadCache("teacherGroupList")
									Client.stackPop(control)
								})
								.fail(JS.failMessage("Törlés sikertelen"))
							},
							text: qsTr("Biztosan törlöd a csoportot?"),
							title: group.name,
							iconSource: Qaterial.Icons.closeCircle
						})
		}
	}


	Action {
		id: actionGroupRename
		text: qsTr("Átnevezés")
		enabled: group
		icon.source: Qaterial.Icons.renameBox
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Csoport neve"),
														   title: qsTr("Csoport átnevezése"),
														   text: group.name,
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(WebSocket.ApiTeacher, "group/%1/update".arg(group.groupid),
																			   {
																				   name: _text
																			   })
															   .done(function(r){
																   Client.reloadCache("teacherGroupList")
															   })
															   .fail(JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}


	Component.onCompleted: group.reload()*/
}
