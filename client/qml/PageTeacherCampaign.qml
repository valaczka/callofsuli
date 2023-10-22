import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		/*var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}*/

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	//property TeacherGroupList _groupListTeacher: Client.cache("teacherGroupList")

	SortFilterProxyModel {
		id: _sortedGroupListTeacher
		sourceModel: _groupModel
		sorters: [
			StringSorter {
				roleName: "text"
			}
		]

		function reload() {
			_groupModel.clear()

			if (!campaign)
				return

			let l = Client.cache("teacherGroupList")

			for (let i=0; i<l.count; ++i) {
				let g = l.get(i)
				if (!g.active)
					continue
				_groupModel.append({
									   text: g.fullName,
									   id: g.groupid
								   })
			}
		}
	}

	ListModel {
		id: _groupModel
	}



	readonly property string _campaignName: campaign ? campaign.readableName : ""

	title: campaign ? _campaignName : qsTr("Új kihívás")
	subtitle: group ? group.fullName : ""


	appBar.backButtonVisible: true

	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: (swipeView.currentIndex < 2) ? menuDetails.open() : menuResult.open()

		QMenu {
			id: menuDetails

			QMenuItem { action: _actionDuplicate }
			QMenuItem { action: _result.actionRepeat }
			QMenuItem { action: _actionRemove }
		}

		QMenu {
			id: menuResult

			QMenuItem { action: _result.actionStudentEdit }
			QMenuItem { action: _result.actionRepeat }
			QMenuItem { action: _actionResultReload }
		}
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherCampaignDetails {
			id: _details
			group: control.group
			campaign: control.campaign
			mapHandler: control.mapHandler
		}

		TeacherCampaignTiming {
			campaign: control.campaign
			onReloadRequest: _details.reloadCampaign()
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
			model.append({ text: qsTr("Kihívás"), source: Qaterial.Icons.trophyVariantOutline, color: Qaterial.Colors.indigo200 })
			model.append({ text: qsTr("Időzítés"), source: Qaterial.Icons.timer, color: Qaterial.Colors.pink200 })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.chartBar, color: Qaterial.Colors.green200 })
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
		text: qsTr("Kihívás törlése")
		icon.source: Qaterial.Icons.delete_
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(WebSocket.ApiTeacher, "campaign/%1/delete".arg(campaign.campaignid))
								 .done(function(r){
									 group.reload()
									 Client.stackPop(control)
								 })
								 .fail(JS.failMessage(qsTr("Törlés sikertelen")))
							 },
							 text: qsTr("Biztosan törlöd a kihívást?"),
							 title: _campaignName,
							 iconSource: Qaterial.Icons.closeCircle
						 })
	}


	Action {
		id: _actionDuplicate
		enabled: campaign
		text: qsTr("Kettőzés")
		icon.source: Qaterial.Icons.contentDuplicate
		onTriggered: {
			_sortedGroupListTeacher.reload()

			Qaterial.DialogManager.openCheckListView(
						{
							onAccepted: function(indexList)
							{
								if (indexList.length === 0)
									return

								var l = []

								for (let i=0; i<indexList.length; ++i) {
									l.push(_sortedGroupListTeacher.get(indexList[i]).id)
								}

								Client.send(WebSocket.ApiTeacher, "campaign/%1/duplicate".arg(campaign.campaignid), {
												list: l
											})
								.done(function(r){
									Client.snack(qsTr("Hadjárat megkettőzve %1x").arg(r.list ? r.list.length : 0))
								})
								.fail(JS.failMessage("Megkettőzés sikertelen"))
							},
							title: qsTr("Hadjárat megkettőzése"),
							standardButtons: Dialog.Cancel | Dialog.Ok,
							model: _sortedGroupListTeacher
						})
		}
	}





}
