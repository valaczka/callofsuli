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
		if (_view.selectEnabled) {
			_view.unselectAll()
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


	title: exam ? (exam.description != "" ? exam.description : qsTr("Dolgozat #%1").arg(exam.examId)) : ""
	subtitle: group ? group.fullName : ""

	appBar.backButtonVisible: true

	appBar.rightComponent: Qaterial.AppBarButton
	{
		action: _actionDelete
		display: AbstractButton.IconOnly
		/*icon.source: Qaterial.Icons.dotsVertical
		onClicked: _menuDetails.open()

		QMenu {
			id: _menuDetails

			QMenuItem { action: _actionDuplicate }
			QMenuItem { action: _actionRemove }
		}*/
	}

	QScrollable {
		anchors.fill: parent

		Qaterial.Expandable {
			id: _expDetails
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true

			header: QExpandableHeader {
				text: qsTr("Alapadatok")
				icon: Qaterial.Icons.cog
				expandable: _expDetails
			}

			delegate: QFormColumn {
				id: _form

				spacing: 3

				Qaterial.TextField {
					id: _tfName

					width: parent.width
					leadingIconSource: Qaterial.Icons.renameBox
					leadingIconInline: true
					title: qsTr("A dolgozat neve")
					readOnly: !exam || exam.state >= Exam.Active
					helperText: !exam || exam.state >= Exam.Active ? qsTr("A dolgozat már kiosztásra került, a név már nem módosítható") : ""

					trailingContent: Qaterial.TextFieldButtonContainer
					{
						Qaterial.TextFieldClearButton { }

						QTextFieldInPlaceButtons {
							setTo: exam ? exam.description : ""
							onSaveRequest: text => {
											   Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(exam.examId),
														   {
															   description: text
														   })
											   .done(root, function(r){
												   reloadExam()
												   saved()
											   })
											   .fail(root, function(err) {
												   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
												   revert()
											   })
										   }
						}
					}
				}


				QDateTimePicker {
					width: parent.width
					canEdit: exam && exam.state < Exam.Active
					title: qsTr("Időpont")

					helperText: !exam || exam.state >= Exam.Active ? qsTr("A dolgozat már kiosztásra került, az időpont már nem módosítható") : ""

					onSaveRequest: text => {
									   Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(exam.examId),
												   {
													   timestamp: hasDate ? Math.floor(date.getTime()/1000) : -1
												   })
									   .done(root, function(r){
										   reloadExam()
										   saved()
									   })
									   .fail(root, function(err) {
										   Client.messageWarning(err, qsTr("Módosítás sikertelen"))
										   revert()
									   })
								   }

					Component.onCompleted: {
						hasDate = exam.timestamp.getTime()
						if (hasDate)
							setFromDateTime(exam.timestamp)
					}
				}

				QFormComboBox {
					id: _comboMode

					text: qsTr("Mód:")

					spacing: 0

					enabled: exam && exam.state < Exam.Active

					combo.width: Math.min(_form.width-spacing-label.width, Math.max(combo.implicitWidth, 200*Qaterial.Style.pixelSizeRatio))

					inPlaceButtons.setTo: exam ? exam.mode : -1

					valueRole: "value"
					textRole: "text"

					model: [
						{ value: 0, text: qsTr("Papír") },
						{ value: 1, text: qsTr("Digitális") },
						{ value: 2, text: qsTr("Virtuális") }
					]

					inPlaceButtonsVisible: true
					inPlaceButtons.onSaveRequest: text => {
													  Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(exam.examId),
																  {
																	  mode: _comboMode.currentValue
																  })
													  .done(root, function(r){
														  reloadExam()
														  inPlaceButtons.saved()
													  })
													  .fail(root, function(err) {
														  Client.messageWarning(err, qsTr("Módosítás sikertelen"))
														  inPlaceButtons.revert()
													  })
												  }
				}




				QFormComboBox {
					id: _comboMap
					text: qsTr("Pálya:")

					combo.width: Math.min(parent.width-spacing-label.width, Math.max(combo.implicitWidth, 300*Qaterial.Style.pixelSizeRatio))

					enabled: exam && exam.state < Exam.Active

					visible: exam && exam.mode != Exam.ExamVirtual

					valueRole: "uuid"
					textRole: "name"

					model: mapHandler ? mapHandler.mapList : null

					inPlaceButtonsVisible: true
					inPlaceButtons.onSaveRequest: text => {
													  Client.send(HttpConnection.ApiTeacher, "exam/%1/update".arg(exam.examId),
																  {
																	  mapuuid: _comboMap.currentValue
																  })
													  .done(root, function(r){
														  reloadExam()
														  inPlaceButtons.saved()
													  })
													  .fail(root, function(err) {
														  Client.messageWarning(err, qsTr("Módosítás sikertelen"))
														  inPlaceButtons.revert()
													  })
												  }

					Component.onCompleted: {
						if (exam)  {
							inPlaceButtons.set(combo.indexOfValue(exam.mapUuid))
						} else
							inPlaceButtons.set(-1)
					}

				}

			}

		}


		Qaterial.Expandable {
			id: _expCreate
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter
			expanded: true

			header: QExpandableHeader {
				text: qsTr("Létrehozás")
				icon: Qaterial.Icons.cog
				expandable: _expCreate
			}

			delegate: Column {
				width: _expCreate.width
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
					to: _teacherExam.examUserList.count
					value: 10
					visible: exam && exam.mode == Exam.ExamVirtual && exam.state < Exam.Active
					onValueChanged: _actionGenerateVirtual.spinCount = value
				}

				QFormSpinBox {
					id: _pdfFontSize
					anchors.horizontalCenter: parent.horizontalCenter
					text: qsTr("PDF betűméret:")
					from: 7
					to: 11
					value: _actionPDF.pdfFontSize
					visible: exam && exam.mode == Exam.ExamPaper
					onValueChanged: _actionPDF.pdfFontSize = value
				}

				QDashboardGrid {
					anchors.horizontalCenter: parent.horizontalCenter

					QDashboardButton {
						id: _btnSelect
						text: qsTr("Küldetés kiválasztása")

						visible: !_btnGenerate.visible && exam && exam.mode != Exam.ExamVirtual && exam.state < Exam.Active

						icon.source: Qaterial.Icons.selectSearch

						onClicked: {
							let l = _teacherExam.getMissionLevelList()
							_tfMission.description = ""
							_teacherExam.missionUuid = ""
							_teacherExam.level = -1

							if (l.length === 0) {
								Client.messageInfo(qsTr("Nincs megfelelő küldetés a pályán!"), qsTr("Küldetés kiválasztása"))
								return
							}

							Qaterial.DialogManager.openListView(
										{
											onAccepted: function(index)
											{
												if (index < 0)
													return

												let li = l[index]

												_tfMission.description = qsTr("%1 - level %2").arg(li.name).arg(li.level)
												_teacherExam.missionUuid = li.uuid
												_teacherExam.level = li.level
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
						visible: _teacherExam.missionUuid != "" && _teacherExam.level > 0 &&
								 exam && exam.mode != Exam.ExamVirtual
					}

					QDashboardButton {
						action: _actionGenerateVirtual
						visible: exam && exam.mode == Exam.ExamVirtual
					}

					QDashboardButton {
						action: _actionRemove
						visible: exam && exam.mode != Exam.ExamVirtual && exam.state < Exam.Active
					}

					QDashboardButton {
						action: _actionPDF
						visible: exam && exam.mode == Exam.ExamPaper
					}

					QDashboardButton {
						action: _actionStart
						visible: exam && exam.mode != Exam.ExamVirtual
						bgColor: Qaterial.Colors.green600
					}

					QDashboardButton {
						action: _actionScan
						visible: enabled
					}

					QDashboardButton {
						action: _actionGrading
						visible: exam && exam.mode != Exam.ExamVirtual && exam.state == Exam.Active
						bgColor: Qaterial.Colors.amber600
					}

					QDashboardButton {
						action: _actionFinish
						bgColor: Qaterial.Colors.red600
					}

				}
			}
		}

		QExpandableHeader {
			width: _view.width
			anchors.horizontalCenter: parent.horizontalCenter
			text: qsTr("Tanulók")
			icon: Qaterial.Icons.accountGroupOutline
			button.visible: false
			topPadding: 10 * Qaterial.Style.pixelSizeRatio
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
				sourceModel: _teacherExam.examUserList

				sorters: [
					StringSorter {
						roleName: "fullName"
					}
				]
			}

			delegate: QLoaderItemDelegate {
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

				rightSourceComponent: Row {
					Qaterial.LabelHeadline6 {
						visible: exam && exam.mode != Exam.ExamVirtual
						anchors.verticalCenter: parent.verticalCenter
						text: grade ? grade.shortname : ""
						color: Qaterial.Style.accentColor
					}

					Qaterial.Icon {
						visible: exam && exam.mode == Exam.ExamVirtual && picked
						anchors.verticalCenter: parent.verticalCenter
						icon: Qaterial.Icons.checkCircle
						color: Qaterial.Colors.green600
						size: 30 * Qaterial.Style.pixelSizeRatio
						sourceSize: Qt.size(size*2, size*2)
					}
				}

				onClicked: Client.stackPushPage("PageTeacherExamGrading.qml", {
													teacherExam: _teacherExam,
													examUser: examUser
												})
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



	Component {
		id: _cmpExportPdf

		QFileDialog {
			title: qsTr("PDF letöltése")
			filters: [ "*.pdf" ]
			isSave: true
			suffix: ".pdf"
			onFileSelected: file => {
								let config = {
									"file": file,
									"fontSize": _actionPDF.pdfFontSize
								}

								let l = []

								if (_view.selectEnabled) {
									l = _view.getSelected()
									_view.unselectAll()
								}

								if (Client.Utils.fileExists(file)) {
									overrideQuestion(file, l, config)
								} else {
									_teacherExam.createPdf(l, config)
								}

								Client.Utils.settingsSet("folder/pdfExport", modelFolder.toString())
							}

			folder: Client.Utils.settingsGet("folder/pdfExport", "")
		}
	}

	function overrideQuestion(file, l, config) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  _teacherExam.createPdf(l, config)
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("PDF letöltése"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}

	Action {
		id: _actionGenerate
		icon.source: Qaterial.Icons.archiveCog

		text: qsTr("Generálás")

		enabled: _teacherExam.missionUuid != "" && _teacherExam.level > 0 &&
				 exam && exam.state < Exam.Active &&
				 (_view.currentIndex != -1 || _view.selectEnabled)

		onTriggered: {
			_teacherExam.generateExamContent(_view.getSelected())
			_view.unselectAll()
		}
	}

	Action {
		id: _actionGenerateVirtual
		icon.source: Qaterial.Icons.pinwheel

		property int spinCount: 10

		text: qsTr("Sorsolás")

		enabled: exam && exam.state < Exam.Active && exam.mode == Exam.ExamVirtual

		onTriggered: {
			let l = []

			for (let i=0; i<_teacherExam.examUserList.count; ++i) {
				let o = _teacherExam.examUserList.get(i)
				l.push(o)
			}

			if (_view.selectEnabled) {
				l = _view.getSelected()
				_view.unselectAll()
			}

			let clearusers = JS.listGetFields(l, "username")

			Client.stackPushPage("PageTeacherExamRunVirtual.qml", {
									 teacherExam: _teacherExam,
									 userList: clearusers,
									 count: spinCount
								 })
		}
	}

	Action {
		id: _actionRemove
		icon.source: Qaterial.Icons.archiveMinus

		text: qsTr("Eltávolítás")

		enabled: exam && exam.state < Exam.Active &&
				 (_view.currentIndex != -1 || _view.selectEnabled)

		onTriggered: {
			let l = JS.listGetFields(_view.getSelected(), "username")

			if (!l.length)
				return

			Client.send(HttpConnection.ApiTeacher, "exam/%1/content/delete".arg(exam.examId), {
							list: l
						})
			.done(root, function(r){
				_view.unselectAll()
				_teacherExam.reloadExamContent()
				Client.messageInfo(qsTr("Diák dolgozat eltávolítva"), qsTr("Diák dolgozat"))
			})
			.fail(root, JS.failMessage(qsTr("Diák dolgozat törlése sikertelen")))

		}
	}

	Action {
		id: _actionPDF
		icon.source: Qaterial.Icons.filePdf
		enabled: exam && exam.mode == Exam.ExamPaper

		property int pdfFontSize: 8

		text: qsTr("PDF letöltése")

		onTriggered: {
			/*if (Qt.platform.os == "wasm")
				mapEditor.wasmSaveAs(false)
			else*/
			Qaterial.DialogManager.openFromComponent(_cmpExportPdf)
		}
	}

	Action {
		id: _actionStart
		icon.source: Qaterial.Icons.play

		text: qsTr("Indítás")
		enabled: exam && exam.state < Exam.Active

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								_teacherExam.activate()
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
		enabled: exam && exam.mode == Exam.ExamPaper && (exam.state == Exam.Active || exam.state == Exam.Grading)

		text: qsTr("Beolvasás")

		onTriggered: {
			Client.stackPushPage("PageTeacherExamScanner.qml", {
									 handler: mapHandler,
									 group: group,
									 acceptedExamIdList: [exam.examId],
									 subtitle: exam.description
								 })
		}
	}


	Action {
		id: _actionGrading
		icon.source: Qaterial.Icons.penOff

		text: qsTr("Dolgozatírás befejezése")
		enabled: exam && exam.state == Exam.Active

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								_teacherExam.inactivate()
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
		enabled: exam && exam.state < Exam.Finished

		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								_teacherExam.finish()
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




	Action {
		id: _actionDelete

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


	function reloadExam() {
		if (_teacherExam)
			_teacherExam.reload()
	}

	Component.onCompleted: {
		_teacherExam.reloadExamContent()
	}

}
