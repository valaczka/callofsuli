import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import SortFilterProxyModel
import "./JScript.js" as JS

Item
{
	id: control

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null
	property PassList passList: null
	property alias view: view

	property alias actionPassAdd: _actionCreate

	property int _lastPassId: -1

	property var stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}


	TeacherPass {
		id: _teacherPass
		teacherGroup: group
		//onExamListReloadRequest: examList.reload()
	}

	QScrollable {
		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: passList.reload()

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: passList

				/*sorters: [
					FilterSorter {
						ValueFilter {
							roleName: "state"
							value: Exam.Prepare
						}
						priority: 3
					},
					FilterSorter {
						ValueFilter {
							roleName: "state"
							value: Exam.Finished
							inverted: true
						}
						priority: 2
					},
					RoleSorter {
						roleName: "timestamp"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]*/
			}

			delegate: QItemDelegate {
				property Pass pass: model.qtObject
				selectableObject: pass

				highlighted: view.selectEnabled ? selected : ListView.isCurrentItem
				iconSource: {
					if (pass.isActive)
						return pass.childless ? Qaterial.Icons.tagText : Qaterial.Icons.tagMultiple
					else if (pass.endTime.getTime() && pass.endTime.getTime() < new Date().getTime())
						return Qaterial.Icons.tagCheckOutline
					else
						return Qaterial.Icons.tagHidden
				}

				iconColorBase: {
					if (pass.isActive)
						return Qaterial.Colors.green400
					else if (pass.endTime.getTime() && pass.endTime.getTime() < new Date().getTime())
						return Qaterial.Style.iconColor()
					else
						return Qaterial.Style.primaryTextColor()
				}

				textColor: iconColorBase
				secondaryTextColor: pass.isActive ? Qaterial.Style.colorTheme.secondaryText : Qaterial.Style.disabledTextColor()


				text: pass.title != "" ? pass.title : qsTr("Call Pass #%1").arg(pass.passid)
				secondaryText: pass ? (pass.startTime.getTime() ? pass.startTime.toLocaleString(Qt.locale(), "yyyy. MMMM d. – ") : "")
									  + (pass.endTime.getTime() ? pass.endTime.toLocaleString(Qt.locale(), "yyyy. MMMM d.") : "")
									: ""

				onClicked: {
					let o = Client.stackPushPage("PageTeacherPass.qml", {
													 group: control.group,
													 pass: pass,
													 passList: passList
												 })

					if (o)
						_lastPassId = pass.passid
				}
			}

			Qaterial.Menu {
				id: _contextMenu
				QMenuItem { action: view.actionSelectAll }
				QMenuItem { action: view.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionDelete }
			}

			onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
											if (index != -1)
											currentIndex = index
											_contextMenu.popup(mouseX, mouseY)
										}
		}

	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen Call Pass sincsen létrehozva. Hozz létre egyet.")
		iconSource: Qaterial.Icons.tagPlus
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: _actionCreate.trigger()

		enabled: group
		visible: group && !passList.length
	}

	QFabButton {
		visible: view.visible
		action: _actionCreate
	}


	Action {
		id: _actionCreate
		enabled: group
		text: qsTr("Új Call Pass")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/pass/create".arg(group.groupid), {
							//mode: Exam.ExamVirtual
						})
			.done(control, function(r){
				if (r.id) {
					Client.send(HttpConnection.ApiTeacher, "group/%1/pass".arg(group.groupid))
					.done(control, function(rr){
						Client.callReloadHandler("pass", passList, rr.list)
						var o = Client.findOlmObject(passList, "passid", r.id)

						if (o) {
							Client.stackPushPage("PageTeacherPass.qml", {
													 group: control.group,
													 pass: o,
													 passList: passList
												 })

							_lastPassId = o.passid
						}

					})
					.fail(control, JS.failMessage(qsTr("Call Pass lista letöltése sikertelen")))


				}
			})
			.fail(control, JS.failMessage(qsTr("Call Pass létrehozása sikertelen")))
		}
	}


	Action {
		id: _actionDelete
		icon.source: Qaterial.Icons.delete_
		text: qsTr("Törlés")
		enabled: view.currentIndex != 1 || view.selectEnabled
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 Call Passt?"), "title",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiTeacher,  "pass/delete", {
															list: JS.listGetFields(l, "passid")
														})
											.done(control, function(r){
												passList.reload()
												view.unselectAll()
											})
											.fail(control, JS.failMessage("Törlés sikertelen"))

										},
										title: qsTr("Call Pass törlése"),
										iconSource: Qaterial.Icons.delete_
									})

		}
	}

	function selectById() {
		for (let i=0; i<view.model.count; ++i) {
			if (view.model.get(i).passid === _lastPassId)
				view.currentIndex = i
		}
	}

}
