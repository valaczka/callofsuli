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

	//property string closeQuestion: _expGrading.modified ? qsTr("Biztosan eldobod a módosításokat?") : ""


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


	columnWidthFunc: function(col) { return col === 1 ? cellWidth * 1.5 : cellWidth }

	readonly property color _rowColor: Client.Utils.colorSetAlpha(Qaterial.Colors.gray700, 0.3)
	readonly property color _linkedColor: Client.Utils.colorSetAlpha(Qaterial.Colors.green500, 0.3)
	readonly property color _assignedColor: Client.Utils.colorSetAlpha(Qaterial.Colors.orange600, 0.3)

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


	// Eredmények

	delegate: Rectangle {
		required property var result

		color: isPlaceholder ? "transparent" :
							   column > 1 && result.assigned ? _assignedColor :
												 row % 2 ? "transparent" : _rowColor
		implicitWidth: 50
		implicitHeight: 50

		Qaterial.LabelSubtitle2 {
			id: _labelResult
			visible: !isPlaceholder && result.assigned && column > 1
			anchors.centerIn: parent
			text: result.pts

			/*color: result.grade ? Qaterial.Style.accentColor : Qaterial.Colors.lightBlue500*/
		}

		Row {
			spacing: 5
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			anchors.horizontalCenterOffset: -10

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

			text: result.pts+"/"+result.maxPts
			visible: !isPlaceholder && column == 1
		}

		QPlaceholderItem {
			visible: isPlaceholder
			anchors.fill: parent
			widthRatio: 1.0
			heightRatio: 1.0
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

			color: !isPlaceholder && passItem && passItem.linkType === PassItem.LinkNone ? _linkedColor : "transparent"

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


	StackView.onActivated: if (teacherPass) teacherPass.reloadPassResult(pass)
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) if (teacherPass) teacherPass.reloadPassResult(pass)

}
