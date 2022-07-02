import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Résztvevők")
	icon: CosStyle.iconGroup

	toolBarComponent: QToolButton {
		action: actionCampaignFilter
		color: actionCampaignFilter.color
		display: AbstractButton.IconOnly
	}

	property ListModel modelUserList: ListModel {}

	QObjectListView {
		id: userList
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: userList.delegateHeight+10
			height: userList.delegateHeight*0.8
			fillMode: Image.PreserveAspectFit
		}


		rightComponent: Column {
			readonly property bool _showNumbers: userList.width > 700
			QLabel {
				anchors.right: parent.right
				text: model ? "%1 XP".arg(Number(model.sumxp).toLocaleString()) : ""
				font.weight: Font.Normal
				font.pixelSize: userList.delegateHeight*0.4
				color: CosStyle.colorAccent
				leftPadding: 5
			}
			Row {
				id: rw
				anchors.right: parent.right
				spacing: 3

				readonly property real rowHeight: userList.delegateHeight*0.3
				readonly property real fontHeight: rowHeight*0.9
				readonly property int fontWeight: Font.DemiBold


				Row {
					visible: model && model.d3
					QTrophyImage {
						level: 3
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.d3 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}

				Row {
					visible: model && model.t3
					QTrophyImage {
						level: 3
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.t3 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}

				Row {
					visible: model && model.d2
					QTrophyImage {
						level: 2
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.d2 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}

				Row {
					visible: model && model.t2
					QTrophyImage {
						level: 2
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.t2 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}

				Row {
					visible: model && model.d1
					QTrophyImage {
						level: 1
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.d1 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}

				Row {
					visible: model && model.t1
					QTrophyImage {
						level: 1
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model ? model.t1 : ""
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
						visible: _showNumbers
					}
				}


			}
		}



		model: SortFilterProxyModel {
			id: userProxyModel
			sourceModel: modelUserList

			sorters: [
				RoleSorter { roleName: "sumxp"; sortOrder: Qt.DescendingOrder; priority: 2 },
				RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder; priority: 1 }
			]

			proxyRoles: [
				ExpressionRole {
					name: "name"
					expression: model.nickname.length ? model.nickname : model.firstname+" "+model.lastname
				},
				SwitchRole {
					name: "background"
					filters: ValueFilter {
						roleName: "username"
						value: cosClient.userName
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
					}
					defaultValue: "transparent"
				},
				SwitchRole {
					name: "titlecolor"
					filters: ValueFilter {
						roleName: "username"
						value: cosClient.userName
						SwitchRole.value: CosStyle.colorAccentLight
					}
					defaultValue: CosStyle.colorAccentLighter
				}

			]
		}

		modelTitleRole: "name"
		modelTitleColorRole: "titlecolor"
		modelBackgroundRole: "background"

		highlightCurrentItem: false

		onRefreshRequest: reloadList()

		onClicked: {
			var o = userList.model.get(index)
			control.tabPage.pushContent(componentUserScore, {
											username: o.username,
											contentTitle: o.name+" | "+control.tabPage.title
										})
		}

	}

	QIconEmpty {
		id: iconEmpty
		visible: modelUserList.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Egyetlen résztvevő sincs még ebben a csoportban")
		tabContainer: control
	}

	Connections {
		target: studentMaps

		function onUserListGet(jsonData, binaryData) {
			if (!jsonData || jsonData.groupid !== studentMaps.selectedGroupId)
				return

			JS.listModelReplace(modelUserList, jsonData.list)
		}
	}

	Connections {
		target: actionCampaignFilter

		function onCampaignChanged() {
			reloadList()
		}
	}

	Component {
		id: componentUserScore
		StudentGroupScore { }
	}

	function reloadList() {
		studentMaps.send("userListGet", {
							 groupid: studentMaps.selectedGroupId,
							 campaignid: actionCampaignFilter.campaign
						 })
	}

	onPopulated: reloadList()


}










