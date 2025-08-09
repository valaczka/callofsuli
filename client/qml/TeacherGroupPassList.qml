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

	property TeacherPass teacherPass: null

	property var stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}

	property int _lastPassId: -1

	property alias actionCreate: _actionCreate
	property alias actionDelete: _actionDelete
	property alias actionDuplicate: _actionDuplicate
	property alias actionSelectAll: view.actionSelectAll
	property alias actionSelectNone: view.actionSelectNone

	QScrollable {
		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		visible: teacherPass

		refreshEnabled: true
		onRefreshRequest: teacherPass.reload()

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: teacherPass ? teacherPass.passList : null

				sorters: [
					RoleSorter {
						roleName: "childless"
						sortOrder: Qt.DescendingOrder
						priority: 4
					},
					RoleSorter {
						roleName: "isActive"
						sortOrder: Qt.DescendingOrder
						priority: 3
					},
					RoleSorter {
						roleName: "endTime"
						sortOrder: Qt.AscendingOrder
						priority: 2
					},
					RoleSorter {
						roleName: "passid"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]
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
				secondaryText: pass ? (pass.startTime.getTime() ? JS.readableDate(pass.startTime) + " – " : "")
									  + (pass.endTime.getTime() ? JS.readableDate(pass.endTime) : "")
									: ""

				onClicked: {
					let o = Client.stackPushPage("PageTeacherPass.qml", {
													 teacherPass: teacherPass,
													 pass: pass,
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
				QMenuItem { action: _actionDuplicate }
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

		enabled: teacherPass
		visible: teacherPass && !teacherPass.passList.length
	}

	QFabButton {
		visible: view.visible
		action: _actionCreate
	}


	SortFilterProxyModel {
		id: _sortedGroupListTeacher
		sourceModel: 	ListModel {
			id: _groupModel
		}

		sorters: [
			StringSorter {
				roleName: "text"
			}
		]

		function reload() {
			_groupModel.clear()

			if (!teacherPass || !teacherPass.teacherGroup)
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

	Action {
		id: _actionDuplicate
		text: qsTr("Kettőzés")
		icon.source: Qaterial.Icons.contentDuplicate
		enabled: teacherPass && teacherPass.teacherGroup && (view.currentIndex != -1 || view.selectEnabled)
		onTriggered: {
			var l = view.getSelected()

			if (!l.length)
				return

			_sortedGroupListTeacher.reload()

			Qaterial.DialogManager.openCheckListView(
						{
							onAccepted: function(indexList)
							{
								if (indexList.length === 0)
									return

								var dst = []

								for (let i=0; i<indexList.length; ++i) {
									dst.push(_sortedGroupListTeacher.get(indexList[i]).id)
								}

								Client.send(HttpConnection.ApiTeacher, "pass/duplicate", {
												src: JS.listGetFields(l, "passid"),
												list: dst
											})
								.done(control, function(r){
									Client.snack(qsTr("%1 Call Pass megkettőzve %2x").arg(l.length).arg(dst.length))
									teacherPass.reload()
									view.unselectAll()
								})
								.fail(control, JS.failMessage("Megkettőzés sikertelen"))
							},
							title: qsTr("Call Pass megkettőzése"),
							standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
							model: _sortedGroupListTeacher
						})
		}
	}

	Action {
		id: _actionCreate
		enabled: teacherPass && teacherPass.teacherGroup
		text: qsTr("Új Call Pass")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/pass/create".arg(teacherPass.teacherGroup.groupid), {
							//mode: Exam.ExamVirtual
						})
			.done(control, function(r){
				if (r.id) {
					Client.send(HttpConnection.ApiTeacher, "group/%1/pass".arg(teacherPass.teacherGroup.groupid))
					.done(control, function(rr){
						Client.callReloadHandler("pass", teacherPass.passList, rr.list)
						var o = Client.findOlmObject(teacherPass.passList, "passid", r.id)

						if (o) {
							Client.stackPushPage("PageTeacherPass.qml", {
													 pass: o,
													 teacherPass: teacherPass
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
		enabled: view.currentIndex != -1 || view.selectEnabled
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
												teacherPass.reload()
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
