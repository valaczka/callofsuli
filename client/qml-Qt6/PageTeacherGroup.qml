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

			QMenuItem { action: _actionGroupRename }
			QMenuItem { action: _actionGroupRemove }
			QMenuItem { action: _result.actionUserEdit }
			Qaterial.MenuSeparator {}
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

		TeacherGroupExamList {
			id: _examList
			group: control.group
			mapHandler: control.mapHandler
		}

		TeacherGroupResult {
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
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.noteCheck, color: "red" })
			model.append({ text: qsTr("Résztvevők"), source: Qaterial.Icons.accountSupervisor, color: "green" })
			model.append({ text: qsTr("Log"), source: Qaterial.Icons.paperRoll, color: "yellow" })
		}
	}


	Action {
		id: _actionResultReload
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: _result.resultModel.reloadContent()
	}

	Action {
		id: _actionGroupRemove
		text: qsTr("Törlés")
		enabled: group
		icon.source: Qaterial.Icons.delete_
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.send(HttpConnection.ApiTeacher, "group/%1/delete".arg(group.groupid))
								.done(control, function(r){
									Client.reloadCache("teacherGroupList")
									Client.stackPop(control)
								})
								.fail(control, JS.failMessage("Törlés sikertelen"))
							},
							text: qsTr("Biztosan törlöd a csoportot?"),
							title: group.name,
							iconSource: Qaterial.Icons.closeCircle
						})
		}
	}


	Action {
		id: _actionGroupRename
		text: qsTr("Átnevezés")
		enabled: group
		icon.source: Qaterial.Icons.accountMultipleCheck
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Csoport neve"),
														   title: qsTr("Csoport átnevezése"),
														   text: group.name,
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(HttpConnection.ApiTeacher, "group/%1/update".arg(group.groupid),
																			   {
																				   name: _text
																			   })
															   .done(control, function(r){
																   Client.reloadCache("teacherGroupList")
															   })
															   .fail(control, JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}


	Component.onCompleted: group.reload()
}
