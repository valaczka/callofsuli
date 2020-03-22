import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QTabContainer {
	id: control

	property string mapUuid: ""

	property alias mapName: labelMapName.text

	property int delegateHeight: CosStyle.twoLineHeight
	property int delegateWidth: 60
	property real delegateSpacing: 3

	property string _field: "xp"
	property int _order: Qt.DescendingOrder
	property bool _fullname: false

	maximumWidth: -1


	menu: QMenu {
		QMenu {
			title: qsTr("Rendezés")

			QMenu {
				title: qsTr("Név")

				MenuItem { action: actionSortNameAsc }
				MenuItem { action: actionSortNameDesc }
			}

			QMenu {
				title: qsTr("XP")

				MenuItem { action: actionSortXPAsc }
				MenuItem { action: actionSortXPDesc }
			}
		}

		MenuItem { action: actionFullName }
	}


	GameMapModel {
		id: mapModel
	}




	SortFilterProxyModel {
		id: mapProxyModel

		sourceModel: mapModel

		sorters: [
			RoleSorter {
				enabled: _field == "xp"
				roleName: "xp"
				sortOrder: _order
				priority: 2
			},
			StringSorter {
				roleName: "details"
				sortOrder: _field == "name" ? _order : Qt.AscendingOrder
				priority: 1
			}
		]

		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: !_fullname && model.userInfo && model.userInfo.nickname && model.userInfo.nickname.length
							? model.userInfo.nickname
							: model.firstname+"\n"+model.lastname
			}
		]
	}


	Component {
		id: userDelegate

		Item {
			id: delegateItem

			required property var displayData

			readonly property int items: displayData.length

			height: delegateHeight
			width: rw.width


			Row {
				id: rw
				spacing: delegateSpacing

				Repeater {
					model: items

					Rectangle {
						id: rect
						readonly property var itemData: displayData[index]

						anchors.verticalCenter: parent.verticalCenter

						height: delegateHeight
						width: delegateWidth

						color: itemData.isCurrent ? CosStyle.colorAccent : "transparent"

						Loader {
							id: ldr
							anchors.centerIn: parent
							width: parent.width*0.9
							height: parent.height*0.9

							asynchronous: true

							visible: status == Loader.Ready
						}


						onItemDataChanged: {
							if (itemData) {
								if (itemData.num > 0) {
									if (ldr.item) {
										ldr.item.level = itemData.success ? itemData.level : -1
										ldr.item.text = itemData.num
									} else {
										ldr.setSource("QMedalImage.qml", {
														  level: itemData.success ? itemData.level : -1,
														  isDeathmatch: itemData.deathmatch,
														  text: itemData.num,
														  image: ""
													  })
									}
								} else if (ldr.item) {
									ldr.source = ""
								}
							}
						}
					}
				}
			}
		}

	}

	QTabHeader {
		id: tabHeader
		tabContainer: control
		isPlaceholder: true
		anchors.top: parent.top
	}

	QLabel {
		id: labelMapName
		anchors.top: tabHeader.bottom
		anchors.bottom: leftHeaderView.top
		anchors.left: parent.left
		anchors.right: topHeaderView.left

		elide: Text.ElideRight
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter
		font.pixelSize: CosStyle.pixelSize*1.2
		font.weight: Font.Medium
		color: CosStyle.colorAccentLight
		wrapMode: Text.Wrap

		leftPadding: 10
		rightPadding: 10
	}

	Flickable {
		id: topHeaderView
		anchors.top: tabHeader.bottom
		anchors.right: parent.right
		anchors.left: leftHeaderView.right
		height: delegateHeight*2

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.HorizontalFlick

		contentHeight: rwH.height+rwL.height
		contentWidth: rwH.width

		Column {
			spacing: 0

			Row {
				id: rwH
				spacing: delegateSpacing

				Repeater {
					model: mapModel.missionData.missions

					Item {
						id: missionItem
						width: modelData.levels*delegateWidth+delegateSpacing*(modelData.levels-1)
						height: delegateHeight

						Row {
							id: row2
							anchors.centerIn: parent
							spacing: 5

							Image {
								id: imgMedal
								height: delegateHeight*0.8
								width: height
								anchors.verticalCenter: parent.verticalCenter
								source: modelData.medalImage.length ? cosClient.medalIconPath(modelData.medalImage) : ""
								fillMode: Image.PreserveAspectFit
							}

							QLabel {
								anchors.verticalCenter: parent.verticalCenter
								text: modelData.name
								width: Math.min(implicitWidth, missionItem.width-imgMedal.width-row2.spacing)
								elide: Text.ElideRight
								color: CosStyle.colorWarningLight
								font.weight: Font.DemiBold
								font.pixelSize: CosStyle.pixelSize*1.1
							}
						}

						Rectangle {
							anchors.bottom: parent.bottom
							width: parent.width
							height: 1
							gradient: Gradient {
								orientation: Gradient.Horizontal
								GradientStop { position: 0.0; color: "transparent" }
								GradientStop { position: 0.3; color: CosStyle.colorWarningLight }
								GradientStop { position: 0.7; color: CosStyle.colorWarningLight }
								GradientStop { position: 1.0; color: "transparent" }
							}
						}
					}
				}
			}

			Row {
				id: rwL
				spacing: delegateSpacing

				Repeater {
					model: mapModel.missionData.levels

					Item {
						anchors.verticalCenter: parent.verticalCenter

						height: delegateHeight
						width: delegateWidth

						Loader {
							anchors.centerIn: parent
							sourceComponent: QTrophyImage {
								width: delegateWidth*0.8
								height: delegateHeight*0.6
								level: modelData.level
								isDeathmatch: modelData.deathmatch
							}
						}
					}
				}
			}
		}

		onContentXChanged: {
			if (!view.moving)
				view.contentX = contentX
		}
	}





	QVariantMapProxyView {
		id: leftHeaderView
		anchors.left: parent.left
		anchors.top: topHeaderView.bottom
		anchors.bottom: parent.bottom

		width: Math.min(parent.width*0.4, 400)

		boundsBehavior: Flickable.StopAtBounds

		model: mapProxyModel

		delegateHeight: control.delegateHeight

		highlightCurrentItem: false
		mouseAreaEnabled: true
		autoSelectorChange: false

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.userInfo.rankid, model.userInfo.ranklevel, model.userInfo.rankimage) : ""
			width: delegateHeight+5
			height: delegateHeight*0.8
			fillMode: Image.PreserveAspectFit
		}

		rightComponent: QLabel {
			horizontalAlignment: Text.AlignRight
			text: model ? "%1 XP".arg(Number(model.xp).toLocaleString()) : ""
			font.pixelSize: CosStyle.pixelSize*1.1
			font.weight: Font.Normal
			color: CosStyle.colorAccentLighter
			rightPadding: 10
			leftPadding: 5
		}

		contentComponent: QLabel {
			text: model ? model.details : ""
			wrapMode: Text.Wrap
			maximumLineCount: 2
			elide: Text.ElideRight
			font.pixelSize: CosStyle.pixelSize*0.9
			font.weight: Font.DemiBold
			color: CosStyle.colorPrimaryLight
		}


		onContentYChanged: {
			if (!view.moving)
				view.contentY = contentY
		}


		onClicked: {
			//var o = leftHeaderView.model.get(index)
			/*JS.createPage("TeacherGroupMapViewUser", {
							  teacherGroups: teacherGroups,
							  defaultSubTitle: String(o.details).replace("\n", " "),
							  defaultTitle: labelMapName.text,
							  username: o.username,
							  mapUuid: mapUuid,
							  groupid: teacherGroups.selectedGroupId
						  })*/
		}

	}




	ListView {
		id: view

		anchors.top: topHeaderView.bottom
		anchors.left: leftHeaderView.right
		//anchors.right: parent.right
		//anchors.bottom: parent.bottom
		width: Math.min(contentWidth, parent.width-x)
		height: Math.min(contentHeight, parent.height-y)

		model: mapProxyModel

		boundsBehavior: Flickable.StopAtBounds

		contentWidth: mapModel.levelCount ? mapModel.levelCount * delegateWidth + (mapModel.levelCount-1)*delegateSpacing : 0

		flickableDirection: Flickable.HorizontalAndVerticalFlick

		clip: true

		ScrollIndicator.vertical: ScrollIndicator { }
		ScrollIndicator.horizontal: ScrollIndicator { }

		delegate: userDelegate

		onContentXChanged: {
			if (!topHeaderView.moving)
				topHeaderView.contentX = contentX
		}

		onContentYChanged: {
			if (!leftHeaderView.moving)
				leftHeaderView.contentY = contentY
		}
	}



	Rectangle {
		anchors.left: leftHeaderView.right
		anchors.top: leftHeaderView.top
		height: leftHeaderView.height
		width: 1
		border.width: 0
		gradient: Gradient {
			orientation: Gradient.Vertical
			GradientStop { position: 0.0; color: "transparent" }
			GradientStop { position: 0.25; color: CosStyle.colorPrimary }
			GradientStop { position: 0.75; color: CosStyle.colorPrimary }
			GradientStop { position: 1.0; color: "transparent" }
		}
	}

	Rectangle {
		anchors.top: topHeaderView.bottom
		anchors.left: topHeaderView.left
		width: topHeaderView.width
		height: 1
		border.width: 0
		gradient: Gradient {
			orientation: Gradient.Horizontal
			GradientStop { position: 0.0; color: "transparent" }
			GradientStop { position: 0.25; color: CosStyle.colorPrimary }
			GradientStop { position: 0.75; color: CosStyle.colorPrimary }
			GradientStop { position: 1.0; color: "transparent" }
		}
	}



	Timer {
		interval: 3000
		repeat: true
		running: control.tabPage.isCurrentItem && control.isCurrentItem
		triggeredOnStart: true
		onTriggered: {
			if (mapUuid.length && teacherGroups)
				teacherGroups.send("groupTrophyGet", {
									   id: teacherGroups.selectedGroupId,
									   map: mapUuid,
									   withUsers: true,
									   withCurrentGame: true
								   })
		}
	}



	Action {
		id: actionSortNameAsc
		text: qsTr("Növekvő")
		checkable: true
		checked: _field == "name" && _order == Qt.AscendingOrder
		onTriggered: {
			_field = "name"
			_order = Qt.AscendingOrder
		}
	}

	Action {
		id: actionSortNameDesc
		text: qsTr("Csökkenő")
		checkable: true
		checked: _field == "name" && _order == Qt.DescendingOrder
		onTriggered: {
			_field = "name"
			_order = Qt.DescendingOrder
		}
	}

	Action {
		id: actionSortXPAsc
		text: qsTr("Növekvő")
		checkable: true
		checked: _field == "xp" && _order == Qt.AscendingOrder
		onTriggered: {
			_field = "xp"
			_order = Qt.AscendingOrder
		}
	}

	Action {
		id: actionSortXPDesc
		text: qsTr("Csökkenő")
		checkable: true
		checked: _field == "xp" && _order == Qt.DescendingOrder
		onTriggered: {
			_field = "xp"
			_order = Qt.DescendingOrder
		}
	}

	Action {
		id: actionFullName
		text: qsTr("Teljes névvel")
		checkable: true
		checked: _fullname
		onTriggered: {
			_fullname = !_fullname
		}
	}


	Component.onCompleted: {
		teacherGroups.onGroupTrophyGet.connect(mapModel.loadFromServer)
	}

	onMapUuidChanged: {
		teacherGroups.loadMapDataToModel(mapUuid, mapModel)
	}

}

