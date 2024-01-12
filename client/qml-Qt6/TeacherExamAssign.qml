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
			expanded: true

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
					visible: _btnSelect.visible
				}

				QFormSpinBox {
					id: _spinCount
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("Dolgozatok száma:")
					from: 1
					to: teacherExam ? teacherExam.examUserList.count : 10
					value: 10
					visible: _exam && _exam.mode == Exam.ExamVirtual && _exam.state < Exam.Active
					onValueChanged: _actionGenerateVirtual.spinCount = value
				}

				QDashboardGrid {
					anchors.horizontalCenter: parent.horizontalCenter

					QDashboardButton {
						id: _btnSelect
						text: qsTr("Küldetés kiválasztása")

						visible: !_btnGenerate.visible && _exam && _exam.mode != Exam.ExamVirtual && _exam.state < Exam.Active

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
						visible: teacherExam && teacherExam.missionUuid != "" && teacherExam.level > 0 &&
								 _exam && _exam.mode != Exam.ExamVirtual
					}

					QDashboardButton {
						action: _actionGenerateVirtual
						visible: _exam && _exam.mode == Exam.ExamVirtual
					}

					QDashboardButton {
						action: _actionRemove
						visible: _exam && _exam.mode != Exam.ExamVirtual && _exam.state < Exam.Active
					}

					QDashboardButton {
						action: _actionPDF
						visible: _exam && _exam.mode == Exam.ExamPaper
					}

					QDashboardButton {
						action: _actionStart
						visible: _exam && _exam.mode != Exam.ExamVirtual
						bgColor: Qaterial.Colors.green600
					}

					QDashboardButton {
						action: _actionScan
						visible: enabled
					}

					QDashboardButton {
						action: _actionGrading
						visible: _exam && _exam.mode != Exam.ExamVirtual && _exam.state == Exam.Active
						bgColor: Qaterial.Colors.red600
					}

					QDashboardButton {
						action: _actionFinish
						visible: _exam && _exam.mode != Exam.ExamVirtual
						bgColor: Qaterial.Colors.red600
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

				highlighted: selected
				iconSource: examData.length ?
								Qaterial.Icons.paperCutVertical :
								Qaterial.Icons.accountOffOutline

				iconColorBase: examData.length ?
								   Qaterial.Colors.green400 :
								   Qaterial.Style.disabledTextColor()

				textColor: iconColorBase

				text: fullName

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
				QMenuItem { action: _actionGenerateVirtual }
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


	Connections {
		target: teacherExam

		function onVirtualListPicked(list) {
			console.debug("VIRTUAL LIST PICKED", list)
		}
	}

	Action {
		id: _actionGenerate
		icon.source: Qaterial.Icons.archiveCog

		text: qsTr("Generálás")

		enabled: teacherExam && teacherExam.missionUuid != "" && teacherExam.level > 0 &&
				 _exam && _exam.state < Exam.Active &&
				 (_view.currentIndex != -1 || _view.selectEnabled)

		onTriggered: {
			teacherExam.generateExamContent(_view.getSelected())
			_view.unselectAll()
		}
	}

	Action {
		id: _actionGenerateVirtual
		icon.source: Qaterial.Icons.pinwheel

		property int spinCount: 10

		text: qsTr("Sorsolás")

		enabled: _exam && _exam.state < Exam.Active && _exam.mode == Exam.ExamVirtual

		onTriggered: {
			let l = []
			let clearlist = []

			for (let i=0; i<teacherExam.examUserList.count; ++i) {
				let o = teacherExam.examUserList.get(i)
				clearlist.push(o)
				l.push(o)
			}

			if (_view.selectEnabled) {
				l = _view.getSelected()
				_view.unselectAll()
			}


			let clearusers = JS.listGetFields(clearlist, "username")

			Client.send(HttpConnection.ApiTeacher, "exam/%1/content/delete".arg(_exam.examId), {
							list: clearusers
						})
			.done(root, function(r){
				teacherExam.pickUsers(JS.listGetFields(l, "username"), spinCount)
			})
		}
	}

	Action {
		id: _actionRemove
		icon.source: Qaterial.Icons.archiveMinus

		text: qsTr("Törlés")

		enabled: _exam && _exam.state < Exam.Active &&
				 (_view.currentIndex != -1 || _view.selectEnabled)

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
			.fail(root, JS.failMessage(qsTr("Dolgozat törlése sikertelen")))

		}
	}

	Action {
		id: _actionPDF
		icon.source: Qaterial.Icons.filePdf
		enabled: _exam && _exam.mode == Exam.ExamPaper

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

	Action {
		id: _actionStart
		icon.source: Qaterial.Icons.play

		text: qsTr("Indítás")
		enabled: _exam && _exam.state < Exam.Active

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								teacherExam.activate()
							},
							text: qsTr("Biztosan elindítod a dolgozatot?"),
							title: qsTr("Dolgozat indítása"),
							iconSource: Qaterial.Icons.play
						})
		}
	}


	Action {
		id: _actionScan
		icon.source: Qaterial.Icons.scanner
		enabled: _exam && _exam.mode == Exam.ExamPaper && (_exam.state == Exam.Active || _exam.state == Exam.Grading)

		text: qsTr("Beolvasás")

		onTriggered: {
			Client.stackPushPage("PageTeacherExamScanner.qml", {
																	 handler: teacherExam.mapHandler,
																	 group: teacherExam.teacherGroup,
																	 acceptedExamIdList: [_exam.examId],
																	 subtitle: _exam.description
																 })
		}
	}


	Action {
		id: _actionGrading
		icon.source: Qaterial.Icons.penOff

		text: qsTr("Dolgozatírás befejezése")
		enabled: _exam && _exam.state == Exam.Active

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								teacherExam.inactivate()
							},
							text: qsTr("Biztosan befejezet a megírási lehetőséget?"),
							title: qsTr("Dolgozatírás befejezése"),
							iconSource: Qaterial.Icons.penOff
						})
		}
	}


	Action {
		id: _actionFinish
		icon.source: Qaterial.Icons.stopCircleOutline

		text: qsTr("Lezárás")
		enabled: _exam && _exam.state < Exam.Finished

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								teacherExam.finish()
							},
							text: qsTr("Biztosan véglegesen lezárod a dolgozatot?"),
							title: qsTr("Dolgozat lezárása"),
							iconSource: Qaterial.Icons.stopCircleOutline
						})
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
