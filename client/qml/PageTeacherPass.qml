import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (swipeView.currentItem.stackPopFunction !== undefined) {
			if (!swipeView.currentItem.stackPopFunction())
				return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.decrementCurrentIndex()
			return false
		}

		return true
	}

	property TeacherPass teacherPass: null
	property Pass pass: null


	title: pass ? (pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)) : qsTr("Új Call Pass")
	subtitle: teacherPass && teacherPass.teacherGroup ? teacherPass.teacherGroup.fullName : ""

	closeQuestion: swipeView.currentItem.closeQuestion !== undefined ? swipeView.currentItem.closeQuestion : ""


	appBar.backButtonVisible: true

	appBar.rightComponent: Row {
		Qaterial.AppBarButton
		{
			visible: swipeView.currentIndex == 0
			icon.source: Qaterial.Icons.dotsVertical
			onClicked: menuDetails.open()

			QMenu {
				id: menuDetails

				QMenuItem { action: _details.actionGradingCopy }
				QMenuItem { action: _details.actionGradingPaste }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionRemove }
			}
		}

		Qaterial.AppBarButton {
			visible: swipeView.currentIndex == 1 && _result.actionApply.enabled
			action: _result.actionApply
			//icon.source: action.icon.source
			display: AbstractButton.IconOnly
		}

		Qaterial.AppBarButton {
			visible: swipeView.currentIndex == 1 && _result.actionCancel.enabled
			action: _result.actionCancel
			//icon.source: action.icon.source
		}
	}


	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		TeacherPassDetails {
			id: _details
			pass: control.pass
			teacherPass: control.teacherPass
		}

		TeacherPassResult {
			id: _result
			pass: control.pass
			teacherPass: control.teacherPass
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Call Pass"), source: Qaterial.Icons.trophyVariantOutline, color: Qaterial.Colors.indigo200 })
			model.append({ text: qsTr("Eredmények"), source: Qaterial.Icons.chartBar, color: Qaterial.Colors.green200 })
		}
	}


	Action {
		id: _actionRemove

		enabled: pass
		text: qsTr("Call Pass törlése")
		icon.source: Qaterial.Icons.delete_
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(HttpConnection.ApiTeacher, "pass/%1/delete".arg(pass.passid))
								 .done(control, function(r){
									 passList.reload()
									 Client.stackPop(control)
								 })
								 .fail(control, JS.failMessage(qsTr("Törlés sikertelen")))
							 },
							 text: qsTr("Biztosan törlöd a Call Passt?"),
							 title: pass.title,
							 iconSource: Qaterial.Icons.closeCircle
						 })
	}

	Component.onCompleted: {
		if (pass)
			pass.reload(HttpConnection.ApiTeacher)
	}
}
