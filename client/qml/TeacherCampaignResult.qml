import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import SortFilterProxyModel 0.2
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
		acceptedButtons: Qt.LeftButton | Qt.RightButton

		onClicked: {
			if (mouse.button == Qt.RightButton) {
				_contextMenu.currentUser = _model.userAt(row-1)
				_contextMenu.open()
				return
			}

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

		QMenu {
			id: _contextMenu

			property User currentUser: null

			QMenuItem {
				icon.source: Qaterial.Icons.accountPlusOutline
				text: qsTr("Kiosztás")
				enabled: campaign && !campaign.finished && _contextMenu.currentUser
				onClicked: {
					JS.questionDialog(
											 {
												 onAccepted: function()
												 {
													 Client.send(HttpConnection.ApiTeacher, "campaign/%1/user/add/%2"
																 .arg(campaign.campaignid).arg(_contextMenu.currentUser.username))
													 .done(root, function(r){
														 _model.reloadContent()
													 })
													 .fail(root, JS.failMessage(qsTr("Kiosztás sikertelen")))
												 },
												 text: qsTr("Biztosan kiosztod a feladatot?\n%1").arg(_contextMenu.currentUser.fullName),
												 title: campaign.readableName,
												 iconSource: Qaterial.Icons.accountPlusOutline
											 })
				}
			}

			QMenuItem {
				icon.source: Qaterial.Icons.accountMinusOutline
				text: qsTr("Visszavonás")
				enabled: campaign && !campaign.finished && _contextMenu.currentUser
				onClicked: {
					JS.questionDialog(
											 {
												 onAccepted: function()
												 {
													 Client.send(HttpConnection.ApiTeacher, "campaign/%1/user/remove/%2"
																 .arg(campaign.campaignid).arg(_contextMenu.currentUser.username))
													 .done(root, function(r){
														 _model.reloadContent()
													 })
													 .fail(root, JS.failMessage(qsTr("Visszavonás sikertelen")))
												 },
												 text: qsTr("Biztosan visszavonod a feladatot?\n%1").arg(_contextMenu.currentUser.fullName),
												 title: campaign.readableName,
												 iconSource: Qaterial.Icons.accountMinusOutline
											 })
				}
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
					enabled: checked
				}

				Qaterial.LabelHeadline5 {
					id: _resultText
					visible: !isPlaceholder
					anchors.verticalCenter: parent.verticalCenter
					text: result
					color: checked ? Qaterial.Style.accentColor : Qaterial.Style.colorTheme.disabledText
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



	SortFilterProxyModel {
		id: _sortedGradeList
		sourceModel: _gradeModel
		sorters: [
			RoleSorter {
				roleName: "gradeid"
			}
		]

		function reload() {
			_gradeModel.clear()

			_gradeModel.append({
								   text: qsTr("Minden jegy"),
								   id: 0
							   })

			let l = Client.cache("gradeList")

			for (let i=0; i<l.count; ++i) {
				let g = l.get(i)
				_gradeModel.append({
									   text: "%1 (%2)".arg(g.longname).arg(g.shortname),
									   id: g.gradeid
								   })
			}
		}
	}

	ListModel {
		id: _gradeModel
	}


	readonly property Action actionStudentEdit:	Action {
		id: _actionStudentEdit
		enabled: campaign && !campaign.finished
		text: qsTr("Kiosztás törlése")
		icon.source: Qaterial.Icons.accountRemoveOutline
		onTriggered: JS.questionDialog(
						 {
							 onAccepted: function()
							 {
								 Client.send(HttpConnection.ApiTeacher, "campaign/%1/user/clear".arg(campaign.campaignid))
								 .done(root, function(r){
									 _model.reloadContent()
								 })
								 .fail(root, JS.failMessage(qsTr("Kiosztás törlése sikertelen")))
							 },
							 text: qsTr("Biztosan törlöd a meglévő kiosztást?\nEzzel mindenkinek ki lesz osztva."),
							 title: campaign.readableName,
							 iconSource: Qaterial.Icons.accountRemoveOutline
						 })
	}



	readonly property Action actionRepeat: Action {
		id: _actionRepeat
		enabled: campaign && campaign.finished
		text: qsTr("Megismétlés")
		icon.source: Qaterial.Icons.repeat
		onTriggered: {
			_sortedGradeList.reload()

			Qaterial.DialogManager.openCheckListView(
						{
							onAccepted: function(indexList)
							{
								if (indexList.length === 0)
									return

								var l = []

								for (let i=0; i<indexList.length; ++i) {
									l.push(_sortedGradeList.get(indexList[i]).id)
								}

								Client.send(HttpConnection.ApiTeacher, "campaign/%1/duplicate".arg(campaign.campaignid), {
												list: [ campaign.groupid ]
											})
								.done(root, function(r){
									let list = r.list

									if (list.length !== 1) {
										Client.messageError(qsTr("Belső hiba"), qsTr("Hadjárat másolása"))
										return
									}

									Client.send(HttpConnection.ApiTeacher, "campaign/%1/user/copy".arg(list[0]), {
													from: campaign.campaignid,
													gradeList: l
												})
									.done(root, function(r){
										Client.snack(qsTr("Hadjárat megismételve"))
									})
									.fail(root, JS.failMessage("Megismétlés sikertelen"))
								})

							},
							title: qsTr("Hadjárat megismétlése a kapott jegyek szerint"),
							standardButtons: Dialog.Cancel | Dialog.Ok,
							model: _sortedGradeList
						})
		}
	}

}
