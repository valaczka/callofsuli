import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: root

	property TeacherMapHandler handler: null
	property ExamScanData currentScan: _view.currentIndex !== -1 ? _view.modelGet(_view.currentIndex) : null
	property alias acceptedExamIdList: _teacherExam.acceptedExamIdList
	property alias group: _teacherExam.teacherGroup

	title:  _teacherExam.uploadableCount ? qsTr("%1 dolgozat feltöltése").arg(_teacherExam.uploadableCount) : qsTr("Dolgozatok beolvasása")

	stackPopFunction: function() {
		if (_actionSave.modified) {
			_actionCancel.trigger()
			return false
		}

		if (_view.selectEnabled) {
			_view.unselectAll()
			return false
		}

		return true
	}

	appBar.backButtonVisible: true

	appBar.rightComponent: Row {
		Qaterial.AppBarButton {
			action: _actionCancel
			visible: _actionSave.modified
		}

		Qaterial.AppBarButton {
			action: _actionSave
			visible: _actionSave.modified
		}
	}

	TeacherExam {
		id: _teacherExam
	}

	QListView {
		id: _view

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.right: _flickable.left
		anchors.rightMargin: 10

		height: parent.height-Math.max(parent.height*0.25, _buttonRows.implicitHeight)

		clip: true

		autoSelectChange: true

		model: _teacherExam.scanData

		delegate: QItemDelegate {
			id: _delegate
			property ExamScanData scanData: model.qtObject
			selectableObject: scanData

			enabled: !_actionSave.modified
			highlighted: ListView.isCurrentItem

			width: ListView.view.width

			text: {
				if (!scanData)
					return ""

				switch (scanData.state) {
				case ExamScanData.ScanFileLoaded:
					return qsTr("Megnyitás...")
				case ExamScanData.ScanFileReadQR:
					return qsTr("QR-kód keresés...")
				case ExamScanData.ScanFileReadingOMR:
					return qsTr("OMR felismerés..")
				case ExamScanData.ScanFileInvalid:
					return qsTr("[Érvénytelen dolgozat]")
				case ExamScanData.ScanFileError:
					return qsTr("[Hibás tartalom]")
				case ExamScanData.ScanFileFinished:
					return (scanData.username != "" ? scanData.username : qsTr("???")) + " (%1)".arg(scanData.examId)
				}

			}

			secondaryText:  path//scanData && scanData.state == ExamScanData.ScanFileFinished ? scanData.path : ""

			iconColorBase: {
				if (!scanData)
					return Qaterial.Style.disabledTextColor()

				switch (scanData.state) {
				case ExamScanData.ScanFileInvalid:
					return Qaterial.Colors.orange600
				case ExamScanData.ScanFileError:
					return Qaterial.Colors.red600
				case ExamScanData.ScanFileFinished:
					if (scanData.upload && scanData.serverAnswer.length) return Qaterial.Colors.green400
				default:
					Qaterial.Style.iconColor()
				}
			}


			textColor: scanData && scanData.state == ExamScanData.ScanFileFinished ?
						   (scanData.serverAnswer.length && scanData.upload) ? Qaterial.Colors.green400
																			 : Qaterial.Style.colorTheme.primaryText : Qaterial.Style.disabledTextColor()

			secondaryTextColor: scanData && scanData.state == ExamScanData.ScanFileFinished ?
									(scanData.serverAnswer.length && scanData.upload) ? Qaterial.Colors.green400
																					  : Qaterial.Style.colorTheme.secondaryText : Qaterial.Style.disabledTextColor()



			iconSource: {
				if (!scanData)
					return ""

				switch (scanData.state) {
				case ExamScanData.ScanFileLoaded:
					return Qaterial.Icons.loading
				case ExamScanData.ScanFileReadQR:
					return Qaterial.Icons.qrcodeScan
				case ExamScanData.ScanFileReadingOMR:
					return Qaterial.Icons.newspaperCheck
				case ExamScanData.ScanFileInvalid:
					return Qaterial.Icons.alert
				case ExamScanData.ScanFileError:
					return Qaterial.Icons.alertOctagon
				case ExamScanData.ScanFileFinished:
					if (scanData.serverAnswer.length && scanData.upload)
						return Qaterial.Icons.databaseCheck
					else if (scanData.serverAnswer.length)
						return Qaterial.Icons.paperRoll
					else
						return Qaterial.Icons.newspaperVariantOutline
				default:
					return ""
				}
			}

			onClicked: _view.currentIndex = index

			Keys.onDeletePressed: _actionDelete.trigger()

			Keys.onSpacePressed: scanData.upload = !scanData.upload

		}

		Qaterial.Menu {
			id: _contextMenu
			QMenuItem { action: _view.actionSelectAll }
			QMenuItem { action: _view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: _actionUploadOn }
			QMenuItem { action: _actionUploadOff }
			Qaterial.MenuSeparator {}
			QMenuItem { action: _actionDelete }
		}

		onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
										if (index != -1)
										currentIndex = index
										_contextMenu.popup(mouseX, mouseY)
									}

	}

	Qaterial.HorizontalLineSeparator {
		id: _separator
		anchors.left: _view.left
		anchors.right: _view.right
		anchors.top: _view.bottom
		anchors.leftMargin: 10 * Qaterial.Style.pixelSizeRatio
		anchors.rightMargin: 10 * Qaterial.Style.pixelSizeRatio
		anchors.topMargin: 5 * Qaterial.Style.pixelSizeRatio
	}

	Item {
		anchors.left: _view.left
		anchors.right: _view.right
		anchors.top: _separator.bottom
		anchors.bottom: parent.bottom
		anchors.topMargin: 5 * Qaterial.Style.pixelSizeRatio

		Row {
			id: _buttonRows
			anchors.centerIn: parent

			QDashboardButton {
				anchors.verticalCenter: parent.verticalCenter
				action: _actionScan
				visible: _teacherExam.scanState == TeacherExam.ScanIdle
			}

			QDashboardButton {
				anchors.verticalCenter: parent.verticalCenter
				action: _actionUpdate
				visible: _teacherExam.scanState == TeacherExam.ScanFinished
			}

			QDashboardButton {
				anchors.verticalCenter: parent.verticalCenter
				action: _actionUpload
				visible: _teacherExam.scanState == TeacherExam.ScanFinished
			}

			Qaterial.IconLabel {
				anchors.verticalCenter: parent.verticalCenter
				color: Qaterial.Style.iconColor()
				icon.source: Qaterial.Icons.magnifyScan
				icon.width: Qaterial.Style.dashboardButtonSize*0.4
				icon.height: Qaterial.Style.dashboardButtonSize*0.4
				text: qsTr("Beolvasás...")
				visible: _teacherExam.scanState == TeacherExam.ScanRunning
			}

			Qaterial.IconLabel {
				anchors.verticalCenter: parent.verticalCenter
				color: Qaterial.Colors.red400
				icon.source: Qaterial.Icons.alertCircle
				icon.width: Qaterial.Style.dashboardButtonSize*0.4
				icon.height: Qaterial.Style.dashboardButtonSize*0.4
				text: qsTr("Hiba!")
				visible: _teacherExam.scanState == TeacherExam.ScanErrorFileSystem ||
						 _teacherExam.scanState == TeacherExam.ScanErrorOmr ||
						 _teacherExam.scanState == TeacherExam.ScanErrorOmrNotFound
			}
		}


	}

	Flickable
	{
		id: _flickable
		width: _content.width

		anchors.right: _imgColumn.left
		anchors.rightMargin: 10
		anchors.bottom: parent.bottom
		anchors.top: parent.top

		contentWidth: _content.implicitWidth
		contentHeight: _content.implicitHeight
		flickableDirection: Flickable.VerticalFlick

		boundsBehavior: Flickable.StopAtBounds
		boundsMovement: Flickable.StopAtBounds

		clip: true

		Column
		{
			id: _content
			enabled: currentScan

			Repeater {
				id: _rptrQuestion

				model: 50
				delegate: Row {
					id: _row
					readonly property int qNum: index+1
					property alias rptrAnswer: _rptrAnswer

					readonly property string originalAnswer: currentScan && currentScan.result["q"+qNum] !== undefined ?
																 currentScan.result["q"+qNum] : ""

					Qaterial.LabelCaption {
						anchors.verticalCenter: parent.verticalCenter
						text: _row.qNum
						leftPadding: 5 * Qaterial.Style.pixelSizeRatio
						rightPadding: 10 * Qaterial.Style.pixelSizeRatio
						width: 30 * Qaterial.Style.pixelSizeRatio
						horizontalAlignment: Label.AlignHCenter
					}

					Repeater {
						id: _rptrAnswer
						model: 6

						Qaterial.ToggleButton {
							anchors.verticalCenter: parent.verticalCenter
							readonly property string letter: Array("A", "B", "C", "D", "E", "F")[index]
							text: letter

							checked: _row.originalAnswer.includes(letter)

							onToggled: {
								_actionSave.modified = true
							}
						}
					}
				}
			}
		}

		ScrollIndicator.vertical: Qaterial.ScrollIndicator {}
	}

	Column {
		id: _imgColumn
		width: _imgNone.checked ? _imgButtonRows.implicitWidth : (parent.width-_flickable.width)/2
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.top: parent.top

		spacing: 10 * Qaterial.Style.pixelSizeRatio

		Row {
			id: _imgButtonRows
			anchors.horizontalCenter: parent.horizontalCenter

			Qaterial.ToggleButton {
				id: _imgNone
				icon.source: Qaterial.Icons.imageRemoveOutline
			}

			Qaterial.ToggleButton {
				id: _imgPath
				icon.source: Qaterial.Icons.imageOutline
				checked: true
			}

			Qaterial.ToggleButton {
				id: _imgOutput
				icon.source: Qaterial.Icons.imageCheck
			}
		}

		Rectangle {
			visible: !_imgNone.checked
			anchors.horizontalCenter: parent.horizontalCenter
			width: parent.width-(20 * Qaterial.Style.pixelSizeRatio)
			height: parent.height-_imgButtonRows.height-2*parent.spacing
			color: "transparent"//Qaterial.Style.dialogColor
			border.color: _img.status == Image.Error ? Qaterial.Colors.red400 : Qaterial.Style.disabledTextColor()
			border.width: 1

			Qaterial.Icon {
				icon: Qaterial.Icons.imageBroken
				color: Qaterial.Colors.red400
				visible: _img.status == Image.Error
				anchors.centerIn: parent
				size: Qaterial.Style.dashboardButtonSize*0.4
			}

			Image {
				id: _img
				anchors.fill: parent
				anchors.margins: 1
				asynchronous: true
				fillMode: Image.PreserveAspectFit
				source: currentScan ? _imgOutput.checked && currentScan.outputPath != "" ? "file://"+currentScan.outputPath
																						 : _imgPath.checked ? "file://"+currentScan.path
																											: "" : ""

			}
		}
	}

	ButtonGroup {
		buttons: _imgButtonRows.children
	}

	Action {
		id: _actionScan
		text: qsTr("Beolvasás")
		icon.source: Qaterial.Icons.scanner
		enabled: _teacherExam.scanState == TeacherExam.ScanIdle
		//onTriggered: _teacherExam.scanImageDir("/tmp/x")
		onTriggered: {
			/*if (Qt.platform.os == "wasm")
				editor.exportData(MapEditor.ExportExam, "", {
									  missionLevel: missionLevel
								  })
			else*/
			Qaterial.DialogManager.openFromComponent(_cmpFolderSelect)
		}
	}

	Action {
		id: _actionUpdate
		text: qsTr("Frissítés")
		icon.source: Qaterial.Icons.refresh
		enabled: _teacherExam.scanState != TeacherExam.ScanRunning
		onTriggered: _teacherExam.updateFromServerRequest()
		shortcut: "Ctrl+R"
	}

	Action {
		id: _actionUpload
		text: qsTr("Feltöltés")
		icon.source: Qaterial.Icons.uploadOutline
		enabled: _teacherExam.uploadableCount
		shortcut: "F10"
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								_teacherExam.uploadResult()
								_view.unselectAll()
							},
							text: qsTr("Biztosan feltöltesz %1 dolgozatot?").arg(_teacherExam.uploadableCount),
							title: qsTr("Eredmények feltöltése"),
							iconSource: Qaterial.Icons.upload
						})
		}
	}

	Action {
		id: _actionSave
		icon.source: Qaterial.Icons.checkBold
		enabled: currentScan
		property bool modified: false

		shortcut: "Ctrl+S"

		onTriggered:  {
			if (!modified)
				return

			let cs = currentScan.result

			for (let i=0; i<_rptrQuestion.count; ++i) {
				let q = _rptrQuestion.itemAt(i)

				let txt = ""

				for (let j=0; j<q.rptrAnswer.count; ++j) {
					let a = q.rptrAnswer.itemAt(j)
					if (a.checked)
						txt += a.text
				}

				cs["q"+q.qNum] = txt
			}

			currentScan.result = cs
			modified = false
			currentScan.serverAnswer = []
		}
	}


	Action {
		id: _actionCancel
		icon.source: Qaterial.Icons.cancel
		enabled: currentScan

		onTriggered:  {
			if (!_actionSave.modified)
				return

			let cs = currentScan.result
			let tmp = currentScan.result

			for (let i=0; i<_rptrQuestion.count; ++i) {
				let q = _rptrQuestion.itemAt(i)

				cs["q"+q.qNum] = q.originalAnswer
				tmp["q"+q.qNum] = "----"
			}

			currentScan.result = tmp
			currentScan.result = cs

			_actionSave.modified = false
		}
	}


	Action {
		id: _actionDelete
		icon.source: Qaterial.Icons.minus
		text: qsTr("Eltávolítás")
		enabled: _view.currentIndex != 1 || _view.selectEnabled
		onTriggered: {
			if (_view.selectEnabled) {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									_teacherExam.removeSelected()
									_view.unselectAll()
								},
								text: qsTr("Biztosan eltávolítod a kijelölt %1 dolgozatot?").arg(Client.Utils.selectedCount(_teacherExam.scanData)),
								title: qsTr("Eltávolítás"),
								iconSource: Qaterial.Icons.delete_
							})

			} else {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									_teacherExam.remove(_view.modelGet(_view.currentIndex))
									_view.unselectAll()
								},
								text: qsTr("Biztosan eltávolítod a dolgozatot?\n")+_view.modelGet(_view.currentIndex).path,
								title: qsTr("Eltávolítás"),
								iconSource: Qaterial.Icons.delete_
							})
			}

		}
	}

	Action {
		id: _actionUploadOn
		icon.source: Qaterial.Icons.upload
		text: qsTr("Feltöltés")
		enabled: _view.currentIndex != 1 || _view.selectEnabled
		onTriggered: {
			let l = _view.getSelected()
			for (let i=0; i<l.length; ++i)
				l[i].upload = true

			_view.unselectAll()
		}
	}

	Action {
		id: _actionUploadOff
		icon.source: Qaterial.Icons.uploadOff
		text: qsTr("Ne töltse fel")
		enabled: _view.currentIndex != 1 || _view.selectEnabled
		onTriggered: {
			let l = _view.getSelected()
			for (let i=0; i<l.length; ++i)
				l[i].upload = false

			_view.unselectAll()
		}
	}

	Action {
		shortcut: "F7"
		onTriggered: _imgNone.checked = true
	}

	Action {
		shortcut: "F8"
		onTriggered: _imgPath.checked = true
	}

	Action {
		shortcut: "F9"
		onTriggered: _imgOutput.checked = true
	}




	Component {
		id: _cmpFolderSelect

		QFileDialog {
			title: qsTr("Képek beolvasása könyvtárból")
			filters: ["*.jpg", "*.JPG", "*.jpeg", "*.JPEG", "*.png", "*.PNG"]
			isDirectorySelect: true
			onDirectorySelected: dir => {
									 _teacherExam.scanImageDir(dir)
									 Client.Utils.settingsSet("folder/scan", dir.toString())
									 /*if (Client.Utils.fileExists(file))
					overrideQuestion(file, false, MapEditor.ExportExam)
				else
					editor.exportData(MapEditor.ExportExam, file, {
										  missionLevel: missionLevel
									  })
				Client.Utils.settingsSet("folder/mapEditor", modelFolder.toString())*/
								 }

			folder: Client.Utils.settingsGet("folder/scan")
		}
	}

	/*
	function overrideQuestion(file, isNew, type) {
		JS.questionDialog({
							  onAccepted: function()
							  {
								  editor.exportData(type, file, {
														missionLevel: missionLevel
													})
							  },
							  text: qsTr("A fájl létezik. Felülírjuk?\n%1").arg(file),
							  title: qsTr("Mentés másként"),
							  iconSource: Qaterial.Icons.fileAlert
						  })
	}
*/
}
