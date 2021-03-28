import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Résztvevők")
	icon: CosStyle.iconUsers


	SortFilterProxyModel {
		id: scoreProxyModel
		sourceModel: studentMaps.modelUserList

		sorters: [
			RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder; priority: 2 },
			RoleSorter { roleName: "sumxp"; sortOrder: Qt.DescendingOrder; priority: 1 }
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


	QVariantMapProxyView {
		id: scoreList
		anchors.fill: parent

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight
		//numbered: true

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: scoreList.delegateHeight+10
			height: scoreList.delegateHeight*0.7
			fillMode: Image.PreserveAspectFit
		}

		rightComponent: Column {
			QLabel {
				anchors.right: parent.right
				text: model.sumxp+" XP"
				font.weight: Font.Normal
				font.pixelSize: scoreList.delegateHeight*0.4
				color: CosStyle.colorAccent
				leftPadding: 5
			}
			Row {
				id: rw
				anchors.right: parent.right
				spacing: 3

				readonly property real rowHeight: scoreList.delegateHeight*0.3
				readonly property real fontHeight: rowHeight*0.9
				readonly property int fontWeight: Font.DemiBold


				Row {
					visible: model.d3
					QTrophyImage {
						level: 3
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.d3
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}

				Row {
					visible: model.t3
					QTrophyImage {
						level: 3
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.t3
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}

				Row {
					visible: model.d2
					QTrophyImage {
						level: 2
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.d2
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}

				Row {
					visible: model.t2
					QTrophyImage {
						level: 2
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.t2
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}

				Row {
					visible: model.d1
					QTrophyImage {
						level: 1
						isDeathmatch: true
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.d1
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}

				Row {
					visible: model.t1
					QTrophyImage {
						level: 1
						isDeathmatch: false
						anchors.verticalCenter: parent.verticalCenter
						height: rw.rowHeight
						width: rw.rowHeight
					}

					QLabel {
						anchors.verticalCenter: parent.verticalCenter
						text: model.t1
						font.weight: rw.fontWeight
						font.pixelSize: rw.fontHeight
					}
				}


			}
		}

		model: scoreProxyModel
		modelTitleRole: "name"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"
		colorSubtitle: CosStyle.colorAccentDark

		highlightCurrentItem: false

		onRefreshRequest: studentMaps.send("userListGet", { groupid: studentMaps.selectedGroupId })
	}



}



