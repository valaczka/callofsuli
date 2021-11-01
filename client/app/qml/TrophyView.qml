import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QSimpleContainer {
	id: control

	title: ""
	icon: CosStyle.iconTrophy

	enum Mode {
		Map,
		Mission,
		User
	}

	property int mode: TrophyView.Map


	signal refreshRequest()

	property VariantMapModel modelGameList: cosClient.newModel([
																   "mapid",
																   "mapname",
																   "missionid",
																   "missionname",
																   "timestamp",
																   "level",
																   "success",
																   "deathmatch",
																   "duration",
																   "xp"
															   ])


	QVariantMapProxyView {
		id: view

		anchors.fill: parent

		model: SortFilterProxyModel {
			sourceModel: control.modelGameList

			sorters: [
				StringSorter {
					roleName: "timestamp"
					sortOrder: Qt.DescendingOrder
				}
			]

			proxyRoles: [
				ExpressionRole {
					name: "titleMap"
					expression: model.mapname+(model.missionname !== "" ? " | "+model.missionname : "")
								+" [%1%2]".arg(model.level).arg(model.deathmatch ? "D" : "")
				},
				ExpressionRole {
					name: "titleMission"
					expression: model.missionname+" [%1%2]".arg(model.level).arg(model.deathmatch ? "D" : "")
				},
				ExpressionRole {
					name: "details"
					expression: "%1 (%2)".arg(model.timestamp).arg(model.duration)
				},
				SwitchRole {
					name: "color"
					ValueFilter {
						roleName: "success"
						value: true
						SwitchRole.value: CosStyle.colorOKLighter
					}
					defaultValue: CosStyle.colorPrimaryLight
				},
				SwitchRole {
					name: "weight"
					ValueFilter {
						roleName: "success"
						value: true
						SwitchRole.value: Font.DemiBold
					}
					defaultValue: Font.Light
				},
				SwitchRole {
					name: "background"
					filters: ValueFilter {
						roleName: "username"
						value: cosClient.userName
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorWarningDark, 0.4)
					}
					defaultValue: "transparent"
				}

			]
		}

		delegateHeight: CosStyle.twoLineHeight

		highlightCurrentItem: false
		mouseAreaEnabled: false
		autoSelectorChange: false

		modelTitleRole: if (control.mode === TrophyView.Map)
							"titleMap"
						else if (control.mode === TrophyView.Mission)
							"titleMission"
						else
							""
		modelSubtitleRole: "details"
		modelTitleColorRole: "color"
		modelSubtitleColorRole: "color"
		modelTitleWeightRole: "weight"
		modelBackgroundRole: "background"

		leftComponent: QTrophyImage {
			width: view.delegateHeight+5
			height: view.delegateHeight*0.8
			isDeathmatch: model && model.succes ? model.deathmatch : false
			level: model && model.success ? model.level : -1
			visible: model
			opacity: model && model.success ? 1.0 : 0.3
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

		refreshEnabled: true
		onRefreshRequest: control.refreshRequest()
	}
}
