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
	property ExamList examList: null
	property alias view: view

	property alias actionExamAdd: _actionCreate

	property int _lastExamId: -1

	property var stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}

		return true
	}


	TeacherExam {
		id: _teacherExam
		teacherGroup: group
		onExamListReloadRequest: examList.reload()
	}

	QScrollable {
		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: examList.reload()

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: examList

				sorters: [
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
				]
			}

			delegate: QItemDelegate {
				property Exam exam: model.qtObject
				selectableObject: exam

				highlighted: view.selectEnabled ? selected : ListView.isCurrentItem
				iconSource: {
					switch (mode) {
					case Exam.ExamOnline:
						return Qaterial.Icons.laptopWindows
					case Exam.ExamPaper:
						return Qaterial.Icons.fileDocumentEdit
					case Exam.ExamVirtual:
						return Qaterial.Icons.stickerTextOutline
					default:
						return Qaterial.Icons.alert
					}
				}

				iconColorBase: {
					switch (exam ? exam.state : 0) {
					case Exam.Finished:
						return Qaterial.Style.iconColor()
					case Exam.Active:
						return Qaterial.Colors.green400
					case Exam.Grading:
						return Qaterial.Colors.orange400
					case Exam.Assigned:
						return Qaterial.Style.primaryTextColor()
					default:
						return Qaterial.Style.disabledTextColor()
					}
				}

				textColor: iconColorBase
				secondaryTextColor: exam && exam.state === Exam.Finished ?
										Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText


				text: description != "" ? description : qsTr("Dolgozat #%1").arg(examId)
				secondaryText: {
					if (timestamp.getTime()) {
						return timestamp.toLocaleString(Qt.locale(), "yyyy. MMMM d. HH:mm")
					}

					return ""
				}

				onClicked: {
					let o = Client.stackPushPage("PageTeacherExam.qml", {
													group: control.group,
													exam: exam,
													mapHandler: control.mapHandler,
													examList: examList
												})

					if (o)
						_lastExamId = examId
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
		text: qsTr("Még egyetlen dolgozat sincsen létrehozva. Hozz létre egyet.")
		iconSource: Qaterial.Icons.trophyVariantOutline
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Létrehozás")

		onAction1Clicked: _actionCreate.trigger()

		enabled: group
		visible: group && !examList.length
	}

	QFabButton {
		visible: view.visible
		action: _actionCreate
	}


	Action {
		id: _actionCreate
		enabled: group
		text: qsTr("Új dolgozat")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/exam/create".arg(group.groupid), {
							mode: Exam.ExamVirtual
						})
			.done(control, function(r){
				if (r.id) {
					Client.send(HttpConnection.ApiTeacher, "group/%1/exam".arg(group.groupid))
					.done(control, function(rr){
						Client.callReloadHandler("exam", examList, rr.list)
						var o = Client.findOlmObject(examList, "examId", r.id)

						if (o) {
							Client.stackPushPage("PageTeacherExam.qml", {
													 group: control.group,
													 exam: o,
													 mapHandler: control.mapHandler,
													 examList: examList
												 })

							_lastExamId = o.examId
						}

					})
					.fail(control, JS.failMessage(qsTr("Dolgozatlista letöltése sikertelen")))


				}
			})
			.fail(control, JS.failMessage(qsTr("Dolgozat létrehozása sikertelen")))
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

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 dolgozatot?"), "description",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiTeacher,  "exam/delete", {
															list: JS.listGetFields(l, "examId")
														})
											.done(control, function(r){
												examList.reload()
												view.unselectAll()
											})
											.fail(control, JS.failMessage("Törlés sikertelen"))

										},
										title: qsTr("Dolgozatok törlése"),
										iconSource: Qaterial.Icons.delete_
									})

		}
	}

	function selectById() {
		for (let i=0; i<view.model.count; ++i) {
			if (view.model.get(i).examId === _lastExamId)
				view.currentIndex = i
		}
	}

}
