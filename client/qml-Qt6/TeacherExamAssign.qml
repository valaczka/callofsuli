import QtQuick
import QtQuick.Controls
import SortFilterProxyModel
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

Item {
	id: root

	property TeacherExam teacherExam: null
	readonly property Exam _exam: teacherExam ? teacherExam.exam : null

	property var stackPopFunction: function() {
		if (_view.selectEnabled) {
			_view.unselectAll()
			return false
		}

		return true
	}

	QScrollable {
		anchors.fill: parent

		Qaterial.Expandable {
			id: _expPaper
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			header: QExpandableHeader {
				text: qsTr("Létrehozás")
				icon: Qaterial.Icons.cog
				expandable: _expPaper
			}

			delegate: Column {
				width: _expPaper.width
				bottomPadding: 30 * Qaterial.Style.pixelSizeRatio

				Qaterial.LabelHeadline5 {
					id: _tfMission
					property string description: ""
					text: description != "" ? description : qsTr("Válassz ki egy küldetést...")
					anchors.horizontalCenter: parent.horizontalCenter
					padding: 10 * Qaterial.Style.pixelSizeRatio
				}

				QDashboardGrid {
					anchors.horizontalCenter: parent.horizontalCenter

					QDashboardButton {
						text: qsTr("Küldetés kiválasztása")

						visible: !_btnGenerate.visible

						icon.source: Qaterial.Icons.selectSearch

						onClicked: {
							let l = teacherExam.getMissionLevelList()
							_tfMission.description = ""
							teacherExam.missionUuid = ""
							teacherExam.level = -1

							if (l.length === 0)
								return

							Qaterial.DialogManager.openListView(
										{
											onAccepted: function(index)
											{
												if (index < 0)
													return

												let li = l[index]

												_tfMission.description = qsTr("%1 - level %2").arg(li.name).arg(li.level)
												teacherExam.missionUuid = li.uuid
												teacherExam.level = li.level
											},
											title: qsTr("Küldetés kiválasztása"),
											model: l,
											delegate: _delegate
										})

						}
					}



					QDashboardButton {
						id: _btnGenerate
						action: _actionGenerate
						visible: teacherExam && teacherExam.missionUuid != "" && teacherExam.level > 0
					}

					QDashboardButton {
						action: _actionRemove
					}

					QDashboardButton {
						action: _actionPDF
						visible: _exam && _exam.mode == 0
					}

				}
			}
		}


		QListView {
			id: _view

			currentIndex: -1
			autoSelectChange: true

			height: contentHeight
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			boundsMovement: Flickable.StopAtBounds
			boundsBehavior: Flickable.StopAtBounds

			model: SortFilterProxyModel {
				sourceModel: teacherExam.examUserList

				sorters: [
					StringSorter {
						roleName: "fullName"
					}
				]
			}

			delegate: QItemDelegate {
				property ExamUser examUser: model.qtObject
				selectableObject: examUser

				highlighted: ListView.isCurrentItem
				iconSource: examUser && examUser.examData.length ?
								Qaterial.Icons.paperCutVertical :
								Qaterial.Icons.accountOffOutline

				iconColorBase: examUser && examUser.examData.length ?
								   Qaterial.Colors.green400 :
								   Qaterial.Style.disabledTextColor()

				textColor: iconColorBase

				text: examUser ? examUser.fullName : ""

				/*onClicked: Client.stackPushPage("PageTeacherExam.qml", {
													group: control.group,
													exam: exam,
													mapHandler: control.mapHandler,
													examList: _examList
												})*/
			}

			Qaterial.Menu {
				id: _contextMenu
				QMenuItem { action: _view.actionSelectAll }
				QMenuItem { action: _view.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: _actionGenerate }
				QMenuItem { action: _actionRemove }
				QMenuItem { action: _actionPDF }
			}

			onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
											if (index != -1)
											currentIndex = index
											_contextMenu.popup(mouseX, mouseY)
										}
		}

	}


	Action {
		id: _actionGenerate
		icon.source: Qaterial.Icons.archiveCog

		text: qsTr("Generálás")

		enabled: teacherExam && teacherExam.missionUuid != "" && teacherExam.level > 0 &&
				 (_view.currentIndex != -1 || _view.selectEnabled)

		onTriggered: {
			teacherExam.generateExamContent(_view.getSelected())
			_view.unselectAll()
		}
	}

	Action {
		id: _actionRemove
		icon.source: Qaterial.Icons.archiveMinus

		text: qsTr("Törlés")

		enabled: _view.currentIndex != -1 || _view.selectEnabled

		onTriggered: {
			let l = JS.listGetFields(_view.getSelected(), "username")

			if (!l.length)
				return

			Client.send(HttpConnection.ApiTeacher, "exam/%1/content/delete".arg(_exam.examId), {
							list: l
						})
			.done(root, function(r){
				_view.unselectAll()
				teacherExam.reloadExamContent()
				Client.messageInfo(qsTr("Dolgozatok törölve"), qsTr("Dolgozat"))
			})
			.fail(root, JS.failMessage(qsTr("Dolgozat letöltése sikertelen")))

		}
	}

	Action {
		id: _actionPDF
		icon.source: Qaterial.Icons.filePdf
		enabled: _exam && _exam.mode == 0

		text: qsTr("PDF letöltése")

		onTriggered: {
			if (_view.selectEnabled) {
				let l = _view.getSelected()

				if (!l.length)
					return

				teacherExam.createPdf(l, {})
				_view.unselectAll()
			} else {
				teacherExam.createPdf([], {})
			}
		}
	}


	property Component _delegate: Qaterial.ItemDelegate
	{
		text: modelData.name ? modelData.name : ""
		secondaryText: modelData.level ? qsTr("level %1").arg(modelData.level) : ""
		width: ListView.view.width
		onClicked: ListView.view.select(index)
	}


	Component.onCompleted: {
		teacherExam.reloadExamContent()
	}
}
