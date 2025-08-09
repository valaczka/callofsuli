import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS


QTableView {
	id: control

	property Pass pass: null
	property TeacherPass teacherPass: null

	property string closeQuestion: _model.hasTmpData ? qsTr("Biztosan eldobod a módosításokat?") : ""


	firstColumnWidth: Math.min(250 * Qaterial.Style.pixelSizeRatio, width*0.45) + Client.safeMarginLeft
	firstRowHeight: Math.min(150 * Qaterial.Style.pixelSizeRatio, height*0.33) + Client.safeMarginTop
	columnSpacing: 1
	rowSpacing: 0

	cellHeight: Qaterial.Style.dense ? Qaterial.Style.textTheme.headline5.pixelSize * 1.4 :
									   Math.max(Qaterial.Style.textTheme.headline5.pixelSize * 1.4,
												Qaterial.Style.delegate.implicitHeight(Qaterial.Style.DelegateType.Icon, 1))
	cellWidth: Qaterial.Style.dense ? Qaterial.Style.textTheme.headline5.pixelSize * 1.4 * 1.75:
									  Math.max(Qaterial.Style.textTheme.headline5.pixelSize * 1.4 * 1.75,
											   Qaterial.Style.delegate.implicitHeight(Qaterial.Style.DelegateType.Icon, 1))


	columnWidthFunc: function(col) { return col === 1 ? cellWidth * 1.7 : cellWidth }

	readonly property color _rowColor: Client.Utils.colorSetAlpha(Qaterial.Colors.gray700, 0.3)
	readonly property color _linkedColor: Client.Utils.colorSetAlpha(Qaterial.Colors.green500, 0.3)
	readonly property color _assignedColor: Client.Utils.colorSetAlpha(Qaterial.Colors.orange600, 0.3)

	readonly property alias actionApply: _actionApply
	readonly property alias actionCancel: _actionCancel

	property var stackPopFunction: function() {
		if (_model.hasTmpData) {
			_actionCancel.trigger()
			return false
		}

		return true
	}

	signal rowClicked(int row)
	signal columnClicked(int col)


	Qaterial.HorizontalLineSeparator {
		x: 0
		width: parent.width
		y: firstRowHeight
		visible: teacherPass && teacherPass.userList.count
	}

	model: TeacherPassResultModel {
		id: _model

		teacherPass: control.teacherPass
		mode: TeacherPassResultModel.ModePass
		contentId: control.pass ? control.pass.passid : -1

		onModelReloaded: {
			control.forceLayout()
		}
	}


	mainView.editTriggers: TableView.NoEditTriggers


	// Eredmények

	delegate: MouseArea {
		id: _delegate

		required property var result
		required property User passUser
		required property PassItem passItem

		implicitWidth: 50
		implicitHeight: 50

		acceptedButtons: Qt.LeftButton

		onPressAndHold: {
			if (!result.assigned) {
				Client.snack(qsTr("Nincs kiosztva"))
				return
			}

			if (passItem.linkType != PassItem.LinkNone) {
				Client.snack(qsTr("Link van beállítva"))
				return
			}

			Qaterial.DialogManager.showTextFieldDialog({
														   title: passUser.fullName,
														   textTitle: passItem.description.length ? passItem.description : qsTr("Eredmény módosítása"),
														   text: teacherPass.round(result.pts),
														   helperText: qsTr("Eredmény beállítása (pont vagy százalék)"),
														   standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror) {
																   teacherPass.assignResult(pass, passItem, [passUser], _text)
															   }
														   }
													   })
		}

		onDoubleClicked: {
			if (!result.assigned) {
				Client.snack(qsTr("Nincs kiosztva"))
				return
			}

			if (passItem.linkType != PassItem.LinkNone) {
				Client.snack(qsTr("Link van beállítva"))
				return
			}

			mainView.edit(mainView.index(row, column))
		}

		Rectangle {
			anchors.fill: parent

			color: isPlaceholder ? "transparent" :
								   tmpData != "" ? Qaterial.Colors.green600 :
												   column > 1 && result.assigned ?
													   (passItem && passItem.linkType !== PassItem.LinkNone ? _linkedColor : _assignedColor) :
													   row % 2 ? "transparent" : _rowColor
			Qaterial.LabelSubtitle2 {
				id: _labelResult
				visible: !isPlaceholder && result.assigned && column > 1
				anchors.centerIn: parent
				text: tmpData != "" ? tmpData : teacherPass.round(result.pts)

				//color: tmpData != "" ? Qaterial.Style.iconColor() : Qaterial.Style.colorTheme.primaryText
			}

			Row {
				spacing: 5
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.verticalCenter: parent.verticalCenter
				anchors.horizontalCenterOffset: -25

				visible: !isPlaceholder && /*result.assigned &&*/ column == 1

				Repeater {
					model: result.grades

					delegate: Qaterial.LabelHeadline6 {
						anchors.verticalCenter: parent.verticalCenter
						text: modelData.shortname
						color: Qaterial.Colors.red400
					}
				}
			}

			Qaterial.LabelHint2 {
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 3

				text: teacherPass.round(result.pts)+"/"+result.maxPts
				visible: !isPlaceholder && column == 1
			}

			QPlaceholderItem {
				visible: isPlaceholder
				anchors.fill: parent
				widthRatio: 1.0
				heightRatio: 1.0
			}
		}

		TableView.editDelegate: Qaterial.TextField {
			anchors.fill: parent
			text: tmpData != "" ? tmpData : teacherPass.round(result.pts)

			errorText: qsTr("Érvénytelen")

			virtualRightPadding: 0

			horizontalAlignment: Qt.AlignHCenter
			selectedTextColor: Qaterial.Colors.black
			textColor: Qaterial.Colors.black

			background: Rectangle {
				anchors.fill: parent
				color: Qaterial.Colors.cyan400
			}

			/*validator: DoubleValidator {
				bottom: 0
				decimals: 2
				top: 1000
				locale: "en"
			}*/


			TableView.onCommit: {
				if (text !== "")
					_model.setTmpData(mainView.index(row, column), text)

				Qt.callLater(_delegate.editNext)
			}

			Keys.onEscapePressed: {
				mainView.closeEditor()
			}

			Keys.onBackPressed: {
				mainView.closeEditor()
			}

			Keys.onUpPressed: event => {
								  let next = _model.getNextEditable(mainView.index(row, column), 0, -1)
								  if (next.valid) mainView.edit(next)
								  event.accepted = true
							  }

			Keys.onDownPressed: event => {
									let next = _model.getNextEditable(mainView.index(row, column), 0, 1)
									if (next.valid) mainView.edit(next)
									event.accepted = true
								}


			Keys.onLeftPressed: event => {
									if (cursorPosition < 1) {
										let next = _model.getNextEditable(mainView.index(row, column), -1, 0)
										if (next.valid) mainView.edit(next)
										event.accepted = true
									} else {
										event.accepted = false
									}
								}

			Keys.onRightPressed: event => {
									 if (cursorPosition >= length) {
										 let next = _model.getNextEditable(mainView.index(row, column), 1, 0)
										 if (next.valid) mainView.edit(next)
										 event.accepted = true
									 } else {
										 event.accepted = false
									 }
								 }

			Component.onCompleted: selectAll()
		}

		function editNext() {
			let next = _model.getNextEditable(mainView.index(row, column), 0, 1)
			if (next.valid)
				mainView.edit(next)
		}
	}



	// PassItems

	topHeaderDelegate:  MouseArea {
		id: _area2

		required property PassItem passItem

		implicitWidth: 50
		implicitHeight: 50

		hoverEnabled: true
		acceptedButtons: Qt.LeftButton

		onClicked: {
			columnClicked(column)

			if (passItem) {
				Client.stackPushPage("TeacherPassItemEdit.qml", {
										 pass: pass,
										 passItem: passItem,
										 teacherPass: teacherPass
									 })
			}
		}

		Rectangle {
			anchors.fill: parent

			color: !isPlaceholder && passItem && passItem.linkType !== PassItem.LinkNone ? _linkedColor : "transparent"

			Qaterial.ListDelegateBackground
			{
				anchors.fill: parent
				type: Qaterial.Style.DelegateType.Icon
				lines: 1
				pressed: _area2.pressed
				rippleActive: _area2.containsMouse
				rippleAnchor: _area2
			}

			Qaterial.Label {
				visible: !isPlaceholder
				anchors.centerIn: parent
				width: parent.height-10
				height: parent.width-10
				elide: Text.ElideRight
				wrapMode: Text.Wrap
				horizontalAlignment: column == 1 ? Text.AlignHCenter : Text.AlignLeft
				verticalAlignment: Text.AlignVCenter
				font.family: Qaterial.Style.textTheme.body2.family
				font.pixelSize: column == 1 ? Qaterial.Style.textTheme.body1.pixelSize
											: Qaterial.Style.textTheme.body2.pixelSize
				font.weight: Font.DemiBold
				lineHeight: 0.8
				text: column == 1 ? qsTr("Eredmény") :
									passItem ? passItem.description : ""

				color: column == 1 ? Qaterial.Style.accentColor : Qaterial.Style.iconColor()
				rotation: column == 1 ? 0 : -90
				maximumLineCount: 3
				rightPadding: column == 1 ? 0 : Client.safeMarginTop
			}


			QPlaceholderItem {
				visible: isPlaceholder
				anchors.centerIn: parent
				width: parent.height
				height: parent.width
				horizontalAlignment: Qt.AlignLeft
				heightRatio: 0.6
				rotation: -90
			}
		}
	}




	// Diákok

	leftHeaderDelegate: MouseArea {
		id: _area

		implicitWidth: 50
		implicitHeight: 50

		hoverEnabled: true
		acceptedButtons: Qt.LeftButton

		required property User passUser

		onClicked: {
			rowClicked(row)

			/*let u = _model.userAt(row-1)
			if (u) {
				Client.stackPushPage("PageTeacherGroupUserResult.qml", {
										 group: root.group,
										 user: u,
										 mapHandler: root.mapHandler
									 })
			}*/
		}



		Rectangle {
			anchors.fill: parent

			color: isPlaceholder ? "transparent" :
								   row % 2 ? "transparent" : _rowColor

			Qaterial.ListDelegateBackground
			{
				anchors.fill: parent
				type: Qaterial.Style.DelegateType.Icon
				lines: 1
				pressed: _area.pressed
				rippleActive: _area.containsMouse
				rippleAnchor: _area
			}


			Qaterial.LabelBody1 {
				visible: !isPlaceholder
				anchors.fill: parent
				leftPadding: Math.max(7, Client.safeMarginLeft)
				rightPadding: 7
				elide: Text.ElideRight
				verticalAlignment: Text.AlignVCenter
				maximumLineCount: Qaterial.Style.dense ? 1 : 2
				lineHeight: 0.8
				wrapMode: Text.Wrap
				text: passUser ? passUser.fullName : ""
			}

		}
	}

	Action {
		id: _actionApply
		enabled: _model.hasTmpData
		icon.source: Qaterial.Icons.checkBold
		text: qsTr("Mentés")
		onTriggered: {
			_model.saveTmpData()
		}
	}

	Action {
		id: _actionCancel
		enabled: _model.hasTmpData
		icon.source: Qaterial.Icons.cancel
		text: qsTr("Mégsem")
		onTriggered: {
			_model.clearTmpData()
		}
	}

	StackView.onActivated: if (teacherPass) teacherPass.reloadPassResult(pass)
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) if (teacherPass) teacherPass.reloadPassResult(pass)

}
