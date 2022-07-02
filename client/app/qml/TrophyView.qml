import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtQuick.Controls.Material 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabContainer {
	id: control

	title: ""
	icon: CosStyle.iconTrophy

	enum Mode {
		Map,
		Mission,
		User
	}

	property int mode: TrophyView.Map
	property alias delegateHeight: view.delegateHeight

	property bool canFetchMore: false
	property bool _fetchSignalSent: false


	signal refreshRequest()
	signal fetchMoreRequest()
	signal fetchMoreSuccess()

	property ListModel modelGameList: ListModel {}


	QObjectListView {
		id: view

		anchors.fill: parent

		model: SortFilterProxyModel {
			sourceModel: control.modelGameList

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
					name: "titleUser"
					expression: model.nickname !== "" ? model.nickname : model.firstname+" "+model.lastname
				},
				ExpressionRole {
					name: "detailsUser"
					expression: model.mapname+(model.missionname !== "" ? " | "+model.missionname : "")
								+" [%1%2] %3 (%4)".arg(model.level).arg(model.deathmatch ? "D" : "").arg(model.timestamp).arg(model.duration)
				},
				ExpressionRole {
					name: "details"
					expression: "%1 (%2)".arg(model.timestamp).arg(model.duration)
					// TODO: expression: "%1 (%2)".arg(JS.readableTimestamp(model.timestamp)).arg(model.duration)
				},
				SwitchRole {
					name: "color"
					filters: [
						AllOf {
							ValueFilter {
								roleName: "success"
								value: true
							}
							ValueFilter {
								roleName: "lite"
								value: true
							}
							SwitchRole.value: CosStyle.colorWarning
						},
						ValueFilter {
							roleName: "success"
							value: true
							SwitchRole.value: CosStyle.colorOKLighter
						}
					]
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
						enabled: control.mode === TrophyView.User
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
						else if (control.mode === TrophyView.User)
							"titleUser"
						else
							""
		modelSubtitleRole: if (control.mode === TrophyView.User)
							   "detailsUser"
						   else
							   "details"
		modelTitleColorRole: "color"
		modelSubtitleColorRole: "color"
		modelTitleWeightRole: "weight"
		modelBackgroundRole: "background"

		leftComponent: QTrophyImage {
			width: view.delegateHeight+5
			height: view.delegateHeight*0.8
			isDeathmatch: model && model.success ? model.deathmatch : false
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

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}

		footer: Item {
			id: footerItem
			visible: false
			width: view.width
			height: view.delegateHeight
			BusyIndicator {
				anchors.centerIn: parent
				height: Math.min(CosStyle.pixelSize*2, parent.height)
				width: height
				running: true
				Material.accent: CosStyle.colorPrimary
			}

			Connections {
				target: control

				function onFetchMoreSuccess() {
					footerItem.visible = false
				}
			}

		}

		onVerticalOvershootChanged: if (verticalOvershoot > 10 && canFetchMore && !_fetchSignalSent) {
										footerItem.visible = true
										_fetchSignalSent = true
										fetchMoreRequest()
									} else if (verticalOvershoot <= 0) {
										_fetchSignalSent = false
									}

	}


	onPopulated: view.forceActiveFocus()
}
