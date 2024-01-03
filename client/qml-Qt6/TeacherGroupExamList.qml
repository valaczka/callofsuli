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
	property alias view: view

	property alias actionExamAdd: actionExamAdd

	ExamList {
		id: _examList
	}

	TeacherExam {
		id: _teacherExam
	}

	QScrollable {
		anchors.fill: parent
		topPadding: 0
		leftPadding: 0
		bottomPadding: 0
		rightPadding: 0

		refreshEnabled: true
		onRefreshRequest: group.reload()

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: false

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: _examList

				/*sorters: [
					RoleSorter {
						roleName: "finished"
						sortOrder: Qt.AscendingOrder
						priority: 3
					},
					RoleSorter {
						roleName: "started"
						sortOrder: Qt.DescendingOrder
						priority: 2
					},
					RoleSorter {
						roleName: "startTime"
						sortOrder: Qt.AscendingOrder
						priority: 1
					}
				]*/
			}

			delegate: QItemDelegate {
				property Exam exam: model.qtObject
				selectableObject: exam

				highlighted: ListView.isCurrentItem
				iconSource: {
					if (!exam)
						return ""
					switch (exam.state) {
					case Exam.Finished:
						return Qaterial.Icons.checkBold
					case Exam.Active:
						return Qaterial.Icons.playCircle
					default:
						return Qaterial.Icons.account
					}
				}

				iconColor: {
					if (!exam)
						return Qaterial.Style.disabledTextColor()
					switch (exam.state) {
					case Exam.Finished:
						return Qaterial.Style.iconColor()
					case Exam.Running:
						return Qaterial.Colors.green400
					default:
						return Qaterial.Style.disabledTextColor()
					}
				}

				textColor: iconColor
				secondaryTextColor: !exam || exam.state == Exam.Finished ?
										Qaterial.Style.disabledTextColor() : Qaterial.Style.colorTheme.secondaryText


				text: exam ? (exam.description != "" ? exam.description : qsTr("Dolgozat #%1").arg(exam.examId)) : ""
				secondaryText: {
					if (!exam)
						return ""

					if (exam.timestamp.getTime()) {
						return exam.timestamp.toLocaleString(Qt.locale(), "yyyy. MMM d. HH:mm")
					}

					return ""
				}

				//onPressAndHold: exam.generateRandom(mapHandler, group)

				onPressAndHold: Client.stackPushPage("PageTeacherExamScanner.qml", {
														 handler: mapHandler,
														 group: group,
														 acceptedExamIdList: [exam.examId],
														 subtitle: text
													 })

				onClicked: {
					Client.send(HttpConnection.ApiTeacher, "exam/%1/content".arg(exam.examId))
					.done(control, function(rr){
						_teacherExam.createPdf(rr.list, {
												   examId: exam.examId,
												   title: exam.description,
												   subject: group.fullName
											   }, group)
					})
				}

				/*onClicked: Client.stackPushPage("PageTeacherCampaign.qml", {
													group: control.group,
													campaign: campaign,
													mapHandler: control.mapHandler
												}) */
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

		onAction1Clicked: actionExamAdd.trigger()

		enabled: group
		visible: group && !_examList.length
	}

	QFabButton {
		visible: view.visible
		action: actionExamAdd
	}


	Action {
		id: actionExamAdd
		enabled: group
		text: qsTr("Új dolgozat")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/exam/create".arg(group.groupid))
			.done(control, function(r){
				if (r.id) {
					Client.send(HttpConnection.ApiTeacher, "group/%1/exam".arg(group.groupid))
					.done(control, function(rr){
						Client.callReloadHandler("exam", _examList, rr.list)
						var o = Client.findOlmObject(_examList, "examId", r.id)

						/*if (o)
							Client.stackPushPage("PageTeacherCampaign.qml", {
													 group: control.group,
													 mapHandler: control.mapHandler,
													 campaign: o
												 })*/
					})
					.fail(control, JS.failMessage(qsTr("Dolgozatlista letöltése sikertelen")))


				}
			})
			.fail(control, JS.failMessage(qsTr("Dolgozat létrehozása sikertelen")))
		}
	}


	function reload() {
		if (!group)
			return

		Client.send(HttpConnection.ApiTeacher, "group/%1/exam".arg(group.groupid))
		.done(control, function(r){
			Client.callReloadHandler("exam", _examList, r.list)
		})
		.fail(control, JS.failMessage(qsTr("Dolgozatlista letöltése sikertelen")))
	}

	StackView.onActivated: reload()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) reload()

}
