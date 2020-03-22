import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: control

	title: qsTr("Rangsor")
	icon: CosStyle.iconTrophy

	property alias list: scoreList

	menu: QMenu {
		MenuItem { action: actionRankList }
	}

	signal userSelected(string user)

	property ListModel modelRankList: ListModel {}

	SortFilterProxyModel {
		id: rankProxyModel

		sourceModel: modelRankList

		sorters: [
			RoleSorter { roleName: "rankid" }
		]
		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: model.ranklevel>0 ? "level "+model.ranklevel : ""
			},
			ExpressionRole {
				name: "readableXP"
				expression: (model.xp || model.ranklevel>0) ? Number(model.xp).toLocaleString() +" XP" : ""
			},
			ExpressionRole {
				name: "imgSource"
				expression: cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage)
			}
		]
	}




	SortFilterProxyModel {
		id: scoreProxyModel
		sourceModel: profile.scoreModel
		filters: [
			AllOf {
				ValueFilter {
					roleName: "isTeacher"
					value: false
				}
				ValueFilter {
					roleName: "isAdmin"
					value: false
				}
				ValueFilter {
					id: classFilter
					roleName: "classid"
					enabled: false
				}
			}
		]

		sorters: [
			RoleSorter { roleName: "rankid"; sortOrder: Qt.DescendingOrder; priority: 2 },
			RoleSorter { roleName: "xp"; sortOrder: Qt.DescendingOrder; priority: 1 }
		]

		proxyRoles: [
			ExpressionRole {
				name: "details"
				expression: model.rankname+(model.ranklevel > 0 ? " (lvl "+model.ranklevel+")": "")
			},
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

	QIconEmpty {
		visible: scoreProxyModel.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		tabContainer: control
	}


	QObjectListView {
		id: scoreList
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.bottom: classTabBar.top

		refreshEnabled: true
		delegateHeight: CosStyle.twoLineHeight
		numbered: true

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		leftComponent: QProfileImage {
			source: model && model.picture ? model.picture : ""
			rankId: model ? model.rankid : -1
			rankImage: model ? model.rankimage : ""
			width: scoreList.delegateHeight+10
			height: scoreList.delegateHeight*0.8
		}

		rightComponent: QLabel {
			text: model ? Number(model.xp).toLocaleString()+" XP" : ""
			font.weight: Font.Normal
			font.pixelSize: scoreList.delegateHeight*0.5
			color: CosStyle.colorAccent
			leftPadding: 5
		}

		model: scoreProxyModel
		modelTitleRole: "name"
		modelSubtitleRole: "details"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"
		colorSubtitle: CosStyle.colorAccentDark

		highlightCurrentItem: false

		onRefreshRequest: profile.send(CosMessage.ClassUserInfo, "getAllUser")

		onClicked: {
			userSelected(scoreList.modelObject(index).username)
		}
	}

	QTabBar {
		id: classTabBar
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		position: TabBar.Footer

		Repeater {
			id: classRepeater

			QTabButton {
				text: modelData.classname
				display: AbstractButton.TextOnly
				onCheckedChanged: {
					if (checked) {
						scoreList.positionViewAtBeginning()
						if (modelData.classid === -1) {
							classFilter.enabled = false
						} else {
							classFilter.value = modelData.classid
							classFilter.enabled = true
						}
					}
				}
			}
		}
	}


	Connections {
		target: profile

		function onGetAllUser(jsonData, binaryData) {
			profile.scoreModelUpdate(jsonData.list)
			var clist = jsonData.classlist
			clist.unshift({"classid": -1, "classname": qsTr("Mindenki")})
			classRepeater.model = clist
		}
	}

	onPopulated: profile.send(CosMessage.ClassUserInfo, "getAllUser")


	Component.onCompleted: {
		modelRankList.clear()
		for (var i=0; i<cosClient.rankList.length; i++) {
			var d = cosClient.rankList[i]
			d.xp = Number(d.xp)
			modelRankList.append(d)
		}
	}


	Action {
		id: actionRankList
		icon.source: CosStyle.iconRank
		text: qsTr("Ranglista")
		onTriggered: {
			var d = JS.dialogCreateQml("List", {
										   roles: ["rankid", "rankname", "details", "imgSource"],
										   icon: actionRankList.icon.source,
										   title: qsTr("Ranglista"),
										   selectorSet: false,
										   modelTitleRole: "rankname",
										   modelImageRole: "imgSource",
										   modelSubtitleRole: "details",
										   modelRightTextRole: "readableXP",
										   delegateHeight: CosStyle.twoLineHeight,
										   model: rankProxyModel
									   })

			d.open()
		}
	}

}
