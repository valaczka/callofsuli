import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSimpleContainer {
	id: control

	title: qsTr("Leggyorsabb megoldások")
	icon: CosStyle.iconTrophy

	property int missionIndex: -1
	property int level: -1
	property bool deathmatch: false
	property string missionId: ""

	maximumWidth: 700

	property ListModel modelGameList: ListModel {}


	QObjectListView {
		id: view

		anchors.fill: parent

		model: SortFilterProxyModel {
			sourceModel: control.modelGameList

			sorters: [
				RoleSorter {
					roleName: "duration"
					sortOrder: Qt.AscendingOrder
					priority: 2
				},
				RoleSorter {
					roleName: "success"
					sortOrder: Qt.DescendingOrder
					priority: 1
				}
			]

			proxyRoles: [
				ExpressionRole {
					name: "details"
					expression: model.nickname.length ? model.nickname : model.firstname+" "+model.lastname
				},
				ExpressionRole {
					name: "durationReadable"
					expression: "(%1x)   %2:%3".arg(model.success).arg(String(Math.floor(model.duration/60)).padStart(2, "0")).arg(String(model.duration%60).padStart(2, "0"))
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
					defaultValue: CosStyle.colorPrimaryLighter
				}
			]
		}

		delegateHeight: CosStyle.halfLineHeight

		highlightCurrentItem: false
		mouseAreaEnabled: false
		autoSelectorChange: false

		modelTitleRole: "details"
		modelBackgroundRole: "background"
		modelTitleColorRole: "titlecolor"

		numbered: true

		leftComponent: Image {
			source: model ? cosClient.rankImageSource(model.rankid, model.ranklevel, model.rankimage) : ""
			width: view.delegateHeight+10
			height: view.delegateHeight*0.7
			fillMode: Image.PreserveAspectFit
		}

		rightComponent: QLabel {
			horizontalAlignment: Text.AlignRight
			text: model ? model.durationReadable : ""
			font.pixelSize: CosStyle.pixelSize*0.9
			font.weight: Font.DemiBold
			color: CosStyle.colorWarningLight
			rightPadding: 10
			leftPadding: 5
		}

		refreshEnabled: true
		onRefreshRequest: reloadData()
	}


	onMissionIndexChanged: {
		var x = studentMaps.modelMissionList.get(missionIndex)
		if (Object.keys(x).length) {
			missionId = x.uuid
		} else {
			missionId = ""
		}
	}

	Connections {
		target: studentMaps

		function onGameListUserMissionGet(jsonData, binaryData) {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Lekérdezési hiba"), jsonData.error)
				return;
			}

			JS.listModelReplace(modelGameList, jsonData.list)
		}
	}

	function reloadData() {
		if (missionId != "")
			studentMaps.send("gameListUserMissionGet", {
								 missionid: missionId,
								 level: level,
								 deathmatch: deathmatch,
								 lite: studentMaps.liteMode
							 })
	}


	onPopulated: reloadData()
}
