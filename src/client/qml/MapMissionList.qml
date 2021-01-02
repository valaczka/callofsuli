import QtQuick 2.12
import QtQuick.Controls 2.12
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: qsTr("Küldetések")
	icon: CosStyle.iconUsers

	property alias list: list


	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: studentMaps.modelMissionList
		sorters: [
			StringSorter { roleName: "cname"; priority: 2 },
			RoleSorter { roleName: "type"; priority: 1 },
			StringSorter { roleName: "name"; priority: 0 }
		]
		proxyRoles: [
			SwitchRole {
				name: "textColor"
				filters: [
					ExpressionFilter {
						expression: model.type === 0 && model.lockDepth > 0
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorAccentLighter, 0.5)
					},
					ExpressionFilter {
						expression: model.type === 1 && model.lockDepth > 0
						SwitchRole.value: JS.setColorAlpha(CosStyle.colorPrimary, 0.5)
					},
					ExpressionFilter {
						expression: model.lockDepth === 0 && model.solved
						SwitchRole.value: CosStyle.colorOKLighter
					},
					ExpressionFilter {
						expression: model.type === 0 && model.lockDepth === 0
						SwitchRole.value: CosStyle.colorAccentLighter
					}
				]
				defaultValue: CosStyle.colorPrimary
			},
			SwitchRole {
				name: "fontWeight"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: Font.Light
				}
				defaultValue: Font.Normal
			},
			SwitchRole {
				name: "fontFamily"
				filters: ValueFilter {
					roleName: "type"
					value: 0
					SwitchRole.value: "Rajdhani"
				}
				defaultValue: "HVD Peace"
			}
		]
	}


	QVariantMapProxyView {
		id: list
		anchors.fill: parent

		visible: studentMaps.modelMissionList.count

		model: userProxyModel
		modelTitleRole: "name"
		modelDepthRole: "type"
		modelTitleColorRole: "textColor"
		//modelTitleWeightRole: "fontWeight"
		modelTitleFamilyRole: "fontFamily"

		depthWidth: CosStyle.baseHeight
		pixelSizeTitle: CosStyle.pixelSize*1.2

		autoSelectorChange: false

		refreshEnabled: true

		//delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconLock

			visible: model && model.type === 1 && model.lockDepth>0

			color: model ? model.textColor : CosStyle.colorPrimary
		}

		rightComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: model && model.type === 1 && model.tried ? CosStyle.iconClock3 : CosStyle.iconLock

			visible: model && ((model.type === 0 && model.lockDepth>0) || (model.type === 1 && model.tried && !model.solved))

			color: model ? model.textColor : CosStyle.colorPrimary
		}


		onRefreshRequest: studentMaps.getMissionList()

		onClicked: {
			var o = list.model.get(index)
			if (o.type === 1 && o.lockDepth === 0) {
				studentMaps.missionSelected(list.model.mapToSource(list.currentIndex))
			} else {
				studentMaps.missionSelected(-1)
			}
		}

		onLongPressed: {
			/*selectorSet = true

			var o = list.model.get(index)

			if (o.type === 0) {
				chaptersFilter.enabled = true
			} else if (o.type === 1) {
				objectiveIdFilter.value = o.id
				objectivesFilter.enabled = true
			}*/

			//mapEditor.modelChapterList.select(serverList.model.mapToSource(serverList.currentIndex))
		}



		//onKeyInsertPressed: actionMapNew.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}



}



