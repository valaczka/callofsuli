import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	stackPopFunction: function() {
		var item = swipeView.currentItem

		if (item && item.stackPopFunction !== undefined) {
			return item.stackPopFunction()
		}

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}

	property ExamList examList: null
	property alias group: _teacherExam.teacherGroup
	property alias exam: _teacherExam.exam
	property alias mapHandler: _teacherExam.mapHandler

	TeacherExam {
		id: _teacherExam
	}

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

			if (!exam)
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



	title: exam ? (exam.description != "" ? exam.description : qsTr("Dolgozat #%1").arg(exam.examId)) : ""
	subtitle: group ? group.fullName : ""

	appBar.backButtonVisible: true

	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: _menuDetails.open() //(swipeView.currentIndex < 2) ? menuDetails.open() : menuResult.open()

		QMenu {
			id: _menuDetails

			QMenuItem { action: _actionDuplicate }
			QMenuItem { action: _actionRemove }
		}
/*
		QMenu {
			id: menuResult

			QMenuItem { action: _result.actionStudentEdit }
			QMenuItem { action: _result.actionRepeat }
			QMenuItem { action: _actionResultReload }
		}*/
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherExamDetails {
			teacherExam: _teacherExam
		}

		TeacherExamAssign {
			teacherExam: _teacherExam
		}

		/*TeacherCampaignTiming {
			campaign: control.campaign
			onReloadRequest: _details.reloadCampaign()
		}

		TeacherCampaignResult {
			id: _result
			group: control.group
			campaign: control.campaign
			mapHandler: control.mapHandler
		}*/

		Rectangle {}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Dolgozat"), source: Qaterial.Icons.trophyVariantOutline, color: Qaterial.Colors.indigo200 })
			model.append({ text: qsTr("Kiosztás"), source: Qaterial.Icons.timer, color: Qaterial.Colors.pink200 })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.chartBar, color: Qaterial.Colors.green200 })
		}
	}



	Action {
		id: _actionRemove

		enabled: exam
		text: qsTr("Dolgozat törlése")
		icon.source: Qaterial.Icons.delete_
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(HttpConnection.ApiTeacher, "exam/%1/delete".arg(exam.examId))
								 .done(root, function(r){
									 if (examList)
										 examList.reload()
									 Client.stackPop(root)
								 })
								 .fail(root, JS.failMessage(qsTr("Törlés sikertelen")))
							 },
							 text: qsTr("Biztosan törlöd a dolgozatot?"),
							 title: root.title,
							 iconSource: Qaterial.Icons.closeCircle
						 })
	}


	Action {
		id: _actionDuplicate
		//enabled: campaign
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

								Client.send(HttpConnection.ApiTeacher, "campaign/%1/duplicate".arg(campaign.campaignid), {
												list: l
											})
								.done(root, function(r){
									Client.snack(qsTr("Hadjárat megkettőzve %1x").arg(r.list ? r.list.length : 0))
								})
								.fail(root, JS.failMessage("Megkettőzés sikertelen"))
							},
							title: qsTr("Hadjárat megkettőzése"),
							standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
							model: _sortedGroupListTeacher
						})
		}
	}


}
