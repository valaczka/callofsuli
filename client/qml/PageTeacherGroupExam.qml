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

	ExamList {
		id: _examList

		function reload() {
			if (!group)
				return

			Client.send(HttpConnection.ApiTeacher, "group/%1/exam".arg(group.groupid))
			.done(control, function(r){
				Client.callReloadHandler("exam", _examList, r.list)
				_result.resultModel.reload()
			})
			.fail(control, JS.failMessage(qsTr("Dolgozatlista letöltése sikertelen")))
		}
	}

	appBar.backButtonVisible: true
	/*appBar.rightComponent: Qaterial.AppBarButton
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
	}*/


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherGroupExamList {
			id: _cmpList
			group: control.group
			mapHandler: control.mapHandler
			examList: _examList
		}

		TeacherGroupExamResult {
			id: _result
			group: control.group
			mapHandler: control.mapHandler
			examList: _examList
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
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.fileDocumentMultiple, color: "red" })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.accountSupervisor, color: "green" })
		}
	}


	Action {
		id: _actionResultReload
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: _examList.reload()
	}

	StackView.onActivated: {
		_cmpList.selectById()
		_examList.reload()
	}
}
