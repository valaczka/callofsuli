import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS


QTableView {
	id: root

	property TeacherGroup group: null
	property Campaign campaign: null
	property TeacherMapHandler mapHandler: null

	property alias resultModel: _model

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

	columnWidthFunc: function(col) { return _model.isSection(col) ? Qaterial.Style.textTheme.caption.pixelSize * 2 : cellWidth }

	readonly property color _rowColor: Client.Utils.colorSetAlpha(Qaterial.Colors.gray700, 0.3)
	readonly property color _sectionColor: Client.Utils.colorSetAlpha(Qaterial.Colors.cyan500, 0.3)

	signal rowClicked(int row)

	Qaterial.HorizontalLineSeparator {
		x: 0
		width: parent.width
		y: firstRowHeight
	}


	model: TeacherGroupCampaignResultModel {
		id: _model
		teacherGroup: root.group
		campaign: root.campaign
		mapHandler: root.mapHandler

		onModelReloaded: {
			root.forceLayout()
		}

	}


	// Eredmények

	delegate: Rectangle {
		color: isPlaceholder ? "transparent" :
							   isSection ? _sectionColor :
										   row % 2 ? "transparent" : _rowColor
		implicitWidth: 50
		implicitHeight: 50
		Qaterial.Icon {
			visible: !isPlaceholder && checked === 1
			anchors.centerIn: parent
			icon: Qaterial.Icons.checkCircle
			color: Qaterial.Colors.green500
			size: Math.min(parent.width, parent.height)*0.8
			sourceSize: Qt.size(size*2, size*2)
		}
		Qaterial.Icon {
			visible: !isPlaceholder && checked === 2
			anchors.centerIn: parent
			icon: Qaterial.Icons.closeThick
			color: Qaterial.Colors.red500
			size: Math.min(parent.width, parent.height)*0.6
			sourceSize: Qt.size(size*2, size*2)

		}
		QPlaceholderItem {
			visible: isPlaceholder
			anchors.fill: parent
			widthRatio: 1.0
			heightRatio: 1.0
		}
	}



	// Taskok

	topHeaderDelegate: Rectangle {
		color: !isPlaceholder && isSection ? _sectionColor : "transparent"
		implicitWidth: 50
		implicitHeight: 50
		Qaterial.Label {
			visible: !isPlaceholder && !isSection
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
		}

		Qaterial.LabelCaption {
			visible: !isPlaceholder && isSection
			anchors.centerIn: parent
			width: parent.height-10
			height: parent.width
			elide: Text.ElideRight
			wrapMode: Text.Wrap
			horizontalAlignment: Text.AlignLeft
			verticalAlignment: Text.AlignVCenter
			lineHeight: 0.8
			text: display
			color: Qaterial.Colors.white
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
				let d = {
					user: u,
					campaignResultModel: _model,
					teacherMapHandler: mapHandler,
					withResult: true,
					title: u.fullName
				}


				if (Qt.platform.os != "ios")					// IOS bug on iPhone 5S
					d.subtitle = group ? group.name : ""

				Client.stackPushPage("PageStudentCampaign.qml", d)
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

			Row {
				spacing: 5
				anchors.fill: parent
				anchors.leftMargin: Math.max(7, Client.safeMarginLeft)
				anchors.rightMargin: 7

				Qaterial.LabelBody1 {
					visible: !isPlaceholder
					anchors.verticalCenter: parent.verticalCenter
					width: parent.width-_resultText.width-parent.spacing
					elide: Text.ElideRight
					maximumLineCount: Qaterial.Style.dense ? 1 : 2
					lineHeight: 0.8
					wrapMode: Text.Wrap
					text: display
				}

				Qaterial.LabelHeadline5 {
					id: _resultText
					visible: !isPlaceholder
					anchors.verticalCenter: parent.verticalCenter
					text: result
					color: Qaterial.Style.accentColor
				}

				QPlaceholderItem {
					visible: isPlaceholder
					width: parent.width
					height: parent.height
					heightRatio: 0.8
					horizontalAlignment: Qt.AlignLeft
				}
			}
		}
	}


	StackView.onActivated: _model.reloadContent()
	SwipeView.onIsCurrentItemChanged: if (SwipeView.isCurrentItem) _model.reloadContent()

}
