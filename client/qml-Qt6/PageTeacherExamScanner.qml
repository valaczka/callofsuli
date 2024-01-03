import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: root

	property TeacherMapHandler handler: null
	property ExamScanData currentScan: _view.currentIndex !== -1 ? _view.modelGet(_view.currentIndex) : null
	property alias acceptedExamIdList: _teacherExam.acceptedExamIdList
	property alias group: _teacherExam.teacherGroup

	title:  _view.uploadableItems ? qsTr("%1 dolgozat feltöltése").arg(_view.uploadableItems) : qsTr("Dolgozatok beolvasása")

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

		property int uploadableItems: 0

		delegate: QItemDelegate {
			id: _delegate
			property ExamScanData scanData: model.qtObject
			selectableObject: scanData

			Connections {
				target: scanData

				function onSelectedChanged() {
					_view.recalculateSelected()
				}
			}

			enabled: !_actionSave.modified
			highlighted: ListView.isCurrentItem

			width: ListView.view.width

			text: scanData.state == ExamScanData.ScanFileFinished ?
					  (scanData.username != "" ? scanData.username : qsTr("???")) + " (%1)".arg(scanData.examId) :
					  scanData.path

			secondaryText: scanData.state == ExamScanData.ScanFileFinished ? scanData.path : ""

			iconColor: {
				switch (scanData.state) {
				case ExamScanData.ScanFileInvalid:
					return Qaterial.Colors.orange400
				case ExamScanData.ScanFileError:
					return Qaterial.Colors.red400
				case ExamScanData.ScanFileFinished:
					return Qaterial.Colors.green400
				default:
					Qaterial.Style.iconColor()
				}
			}


			textColor: scanData.state == ExamScanData.ScanFileFinished ?
						   scanData.serverAnswer.length ? Qaterial.Colors.green400
														: Qaterial.Style.colorTheme.primaryText : Qaterial.Style.disabledTextColor()

			secondaryTextColor: scanData.state == ExamScanData.ScanFileFinished ?
									scanData.serverAnswer.length ? Qaterial.Colors.green400
																 : Qaterial.Style.colorTheme.secondaryText : Qaterial.Style.disabledTextColor()



			iconSource: {
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
					if (scanData.serverAnswer.length)
						return Qaterial.Icons.databaseCheck
					else
						return Qaterial.Icons.paperRoll
				default:
					return ""
				}
			}

			onClicked: _view.currentIndex = index

		}

		Keys.onDeletePressed: {
			if (_teacherExam.scanState != TeacherExam.ScanFinished)
				return

			let l = getSelected()

			for (let i=0; i<l.length; ++i) {
				_teacherExam.scanData.remove(i)
			}
		}

		function recalculateSelected() {
			let n = 0
			for (let i=0; i<count; ++i) {
				let o = modelGet(i)
				if (o.selected && o.state === ExamScanData.ScanFileFinished && o.serverAnswer.length)
					++n
			}
			uploadableItems = n
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
		onTriggered: _teacherExam.scanImageDir("/tmp/x")
	}

	Action {
		id: _actionUpdate
		text: qsTr("Update")
		icon.source: Qaterial.Icons.refresh
		enabled: _teacherExam.scanState != TeacherExam.ScanRunning
		onTriggered: _teacherExam.updateFromServerRequest()
		shortcut: "F3"
	}

	Action {
		id: _actionUpload
		text: qsTr("Feltöltés")
		icon.source: Qaterial.Icons.uploadOutline
		enabled: _view.uploadableItems
		//onTriggered: _teacherExam.updateFromServerRequest()
	}

	Action {
		id: _actionSave
		icon.source: Qaterial.Icons.checkBold
		enabled: currentScan
		property bool modified: false

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

		shortcut: "Ctrl+S"

		onTriggered:  {
			if (!_actionSave.modified)
				return

			let cs = currentScan.result

			for (let i=0; i<_rptrQuestion.count; ++i) {
				let q = _rptrQuestion.itemAt(i)

				cs["q"+q.qNum] = q.originalAnswer
			}

			currentScan.result = {}
			currentScan.result = cs

			_actionSave.modified = false
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

}
