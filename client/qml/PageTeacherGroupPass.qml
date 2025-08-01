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

	PassList {
		id: _passList

		function reload() {
			if (!group)
				return

			Client.send(HttpConnection.ApiTeacher, "group/%1/pass".arg(group.groupid))
			.done(control, function(r){
				Client.callReloadHandler("pass", _passList, r.list)
				_result.resultModel.reload()
			})
			.fail(control, JS.failMessage(qsTr("Lista letöltése sikertelen")))
		}
	}

	appBar.backButtonVisible: true

	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherGroupPassList {
			id: _cmpList
			group: control.group
			mapHandler: control.mapHandler
			passList: _passList
		}

		TeacherGroupExamResult {
			id: _result
			/*group: control.group
			mapHandler: control.mapHandler
			examList: _examList*/
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
			model.append({ text: qsTr("Call Pass"), source: "qrc:/internal/img/passIcon.svg", color: Qaterial.Colors.lightBlue400 })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.accountSupervisor, color: Qaterial.Colors.green400 })
		}
	}


	Action {
		id: _actionResultReload
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: _passList.reload()
	}

	StackView.onActivated: {
		_cmpList.selectById()
		_passList.reload()
	}
}
