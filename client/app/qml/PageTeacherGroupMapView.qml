import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: page

	defaultTitle: ""
	defaultSubTitle: ""

	property TeacherGroups teacherGroups: null
	property string mapUuid: ""

	property int delegateHeight: CosStyle.twoLineHeight
	property int delegateWidth: 60
	property real delegateSpacing: 3

	property int _order: Qt.DescendingOrder

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	GameMapModel {
		id: mapModel
	}


	/*mainMenuFunc: function (m) {
		m.addAction(actionMapAdd)
	}*/




	SortFilterProxyModel {
		id: mapProxyModel

		sourceModel: mapModel

		sorters: [
			RoleSorter { roleName: "xp"; ascendingOrder: _order }
		]

		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: model.firstname+" "+model.lastname
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



	Flickable {
		id: topHeaderView
		anchors.top: parent.top
		anchors.right: parent.right
		anchors.left: leftHeaderView.right
		height: delegateHeight*2

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.HorizontalFlick

		contentHeight: rwH.height
		contentWidth: rwH.width

		Column {
			spacing: 0

			Row {
				id: rwH
				spacing: delegateSpacing

				Repeater {
					model: mapModel.missionData.missions

					Rectangle {
						width: modelData.levels*delegateWidth+delegateSpacing*(modelData.levels-1)
						height: delegateHeight
						color: "yellow"

						QLabel {
							anchors.centerIn: parent
							text: modelData.name
						}
					}
				}
			}

			Row {
				id: rwL
				spacing: delegateSpacing

				Repeater {
					model: mapModel.missionData.levels

					Rectangle {
						anchors.verticalCenter: parent.verticalCenter

						height: delegateHeight
						width: delegateWidth

						color: "green"

						QLabel {
							anchors.centerIn: parent
							text: modelData.level+(modelData.deathmatch ? "D" : "")
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





	ListView {
		id: leftHeaderView
		anchors.left: parent.left
		anchors.top: topHeaderView.bottom
		anchors.bottom: parent.bottom

		width: 400

		boundsBehavior: Flickable.StopAtBounds

		clip: true

		model: mapProxyModel

		delegate: Rectangle {
			implicitHeight: delegateHeight
			implicitWidth: leftHeaderView.width
			color: "blue"

			Image {
				source: cosClient.rankImageSource(userInfo.rankid, userInfo.ranklevel, userInfo.rankimage)
				width: parent.height+10
				height: parent.height*0.7
				fillMode: Image.PreserveAspectFit

				anchors.verticalCenter: parent.verticalCenter
			}

			QLabel {
				anchors.centerIn: parent
				text: details+" "+xp+" XP"
			}
		}

		onContentYChanged: {
			if (!view.moving)
				view.contentY = contentY
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


	Timer {
		interval: 3000
		repeat: true
		running: page.isCurrentItem
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



	onMapUuidChanged: {
		if (teacherGroups)
			teacherGroups.loadMapDataToModel(mapUuid, mapModel)
	}

	onTeacherGroupsChanged: {
		if (teacherGroups)
			teacherGroups.onGroupTrophyGet.connect(mapModel.loadFromServer)

		if (mapUuid.length && teacherGroups)
			teacherGroups.loadMapDataToModel(mapUuid, mapModel)
	}


	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}

