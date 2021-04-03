import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QSwipeContainer {
	id: panel

	title: qsTr("Pályák")
	icon: CosStyle.iconPlanet

	property alias list: list

	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: studentMaps.modelMapList
		sorters: [
			RoleSorter { roleName: "active"; sortOrder: Qt.DescendingOrder; priority: 1 },
			StringSorter { roleName: "name"; priority: 0 }
		]

		proxyRoles: [
			SwitchRole {
				name: "textColor"
				filters: [
					ValueFilter {
						roleName: "active"
						value: true
						SwitchRole.value: CosStyle.colorOK
					}
				]
				defaultValue: CosStyle.colorPrimaryLighter
			},
			SwitchRole {
				name: "fontWeight"
				filters: ExpressionFilter {
					expression: model.active
					SwitchRole.value: Font.DemiBold
				}
				defaultValue: Font.Medium
			}
		]
	}


	QVariantMapProxyView {
		id: list

		anchors.fill: parent

		model: userProxyModel
		modelTitleRole: "name"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		autoSelectorChange: true

		refreshEnabled: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: if (model && model.downloaded) {
					  if (model.active)
						  "qrc:/internal/img/battle.png"
					  else
						  CosStyle.iconMedal
				  } else
					  CosStyle.iconDownloadCloud

			visible: model

			color: model && model.downloaded ? model.textColor : CosStyle.colorAccent
		}

		rightComponent: QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			ToolTip.text: qsTr("Jelvények")

			visible: model && model.active && model.downloaded

			icon.source: CosStyle.iconMedal

			onClicked: {
				var d = JS.dialogCreateQml("StudentMapInfo", {
											   title: model.name,
											   studentMaps: studentMaps,
											   mapUuid: model.uuid
										   })
				d.open()
			}
		}



		onRefreshRequest: studentMaps.send("mapListGet", { groupid: studentMaps.selectedGroupId } )

		onClicked: {
			var o = list.model.get(index)
			if (o.downloaded) {
				if (o.active) {
					studentMaps.mapLoad({uuid: o.uuid, name: o.name})
				} else {
					var d = JS.dialogCreateQml("StudentMapInfo", {
												   title: o.name,
												   studentMaps: studentMaps,
												   mapUuid: o.uuid
											   })
					d.open()

				}
			} else {
				list.currentIndex = index
				actionDownload.trigger()
			}
		}

		onRightClicked: contextMenu.popup()

		onLongPressed: {
			if (selectorSet) {
				contextMenu.popup()
				return
			}
		}



		QMenu {
			id: contextMenu

			MenuItem { action: actionDownload }
		}


		//onKeyInsertPressed: actionMapNew.trigger()
		//onKeyF2Pressed: actionRename.trigger()
		/*onKeyDeletePressed: actionRemove.trigger()
		onKeyF4Pressed: actionObjectiveNew.trigger()*/
	}


	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDownload
		enabled: !studentMaps.isBusy && (list.currentIndex !== -1 || studentMaps.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = studentMaps.modelMapList.selectedCount

			if (more > 0)
				studentMaps.mapDownload({list: studentMaps.modelMapList.getSelectedData("uuid") })
			else
				studentMaps.mapDownload({uuid: o.uuid})
		}
	}



}



