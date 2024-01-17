import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QTableView {
	id: root

	property TeacherGroup group: null
	property TeacherMapHandler mapHandler: null

	property alias resultModel: _model
	property alias actionUserEdit: _actionUserEdit

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


	//columnWidthFunc: function(col) { return _model.isSection(col) ? Qaterial.Style.textTheme.caption.pixelSize * 2 : cellWidth }

	readonly property color _rowColor: Client.Utils.colorSetAlpha(Qaterial.Colors.gray700, 0.3)
	readonly property color _runningColor: Client.Utils.colorSetAlpha(Qaterial.Colors.green500, 0.3)

	signal rowClicked(int row)
	signal columnClicked(int col)


	Qaterial.HorizontalLineSeparator {
		x: 0
		width: parent.width
		y: firstRowHeight
		visible: group && group.memberList.length
	}

	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen résztvevő sincsen felvéve. Adj hozzá a csoporthoz egy osztályt vagy felhasználót.")
		iconSource: Qaterial.Icons.accountOutline
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		action1: qsTr("Hozzáadás")

		onAction1Clicked: _actionUserEdit.trigger()

		enabled: group
		visible: group && !group.memberList.length
	}

	QFabButton {
		visible: group && !group.memberList.length
		action: _actionUserEdit
	}

	Action {
		id: _actionUserEdit
		icon.source: Qaterial.Icons.accountEdit
		text: qsTr("Résztvevők")
		onTriggered: {
			Client.stackPushPage("PageTeacherGroupEdit.qml", {
									 group: group
								 })

		}
	}



	model: TeacherGroupResultModel {
		id: _model
		teacherGroup: root.group

		onModelReloaded: {
			root.forceLayout()
		}

	}


	// Eredmények

	delegate: Rectangle {
		color: isPlaceholder ? "transparent" :
							   campaignState === Campaign.Running ? _runningColor :
																	row % 2 ? "transparent" : _rowColor
		implicitWidth: 50
		implicitHeight: 50

		Qaterial.LabelHeadline5 {
			id: _labelResult
			visible: !isPlaceholder && result.grade
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter
			anchors.margins: 3
			text: result.grade ? result.grade.shortname : ""
			color: Qaterial.Style.accentColor

			states: [
				State {
					name: "withXP"
					when: result.xp > 0
					AnchorChanges {
						target: _labelResult
						anchors.left: parent.left
						anchors.horizontalCenter: undefined
						anchors.verticalCenter: undefined
						anchors.top: parent.top
					}
				}
			]
		}

		Qaterial.LabelCaption {
			visible: !isPlaceholder && result.xp > 0
			anchors.bottom: parent.bottom
			anchors.right: parent.right
			anchors.margins: 3

			text: result.xp
		}

		QPlaceholderItem {
			visible: isPlaceholder
			anchors.fill: parent
			widthRatio: 1.0
			heightRatio: 1.0
		}
	}



	// Kihívások


	topHeaderDelegate:  MouseArea {
		id: _area2
		implicitWidth: 50
		implicitHeight: 50

		hoverEnabled: true
		acceptedButtons: Qt.LeftButton

		onClicked: {
			columnClicked(column)

			let c = _model.campaignAt(column-1)
			if (c) {
				Client.stackPushPage("PageTeacherCampaignResult.qml", {
										 group: root.group,
										 campaign: c,
										 mapHandler: root.mapHandler
									 })
			}
		}

		Rectangle {
			anchors.fill: parent

			color: !isPlaceholder && campaignState === Campaign.Running ? _runningColor : "transparent"

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
				horizontalAlignment: Text.AlignLeft
				verticalAlignment: Text.AlignVCenter
				font.family: Qaterial.Style.textTheme.body2.family
				font.pixelSize: Qaterial.Style.textTheme.body2.pixelSize
				font.weight: Font.DemiBold
				lineHeight: 0.8
				text: display
				color: Qaterial.Style.iconColor()
				rotation: -90
				maximumLineCount: 3
				rightPadding: Client.safeMarginTop
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

		onClicked: {
			rowClicked(row)

			let u = _model.userAt(row-1)
			if (u) {
				Client.stackPushPage("PageTeacherGroupUserResult.qml", {
										 group: root.group,
										 user: u,
										 mapHandler: root.mapHandler
									 })
			}
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
				text: display
			}

		}
	}


	StackView.onActivated: _model.reloadContent()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _model.reloadContent()

}
