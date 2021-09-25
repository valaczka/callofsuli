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


	QVariantMapProxyView {
		id: list

		anchors.fill: parent

		model: SortFilterProxyModel {
			sourceModel: teacherGroups.modelMapList
			sorters: [
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

		modelTitleRole: "name"
		modelTitleColorRole: "textColor"
		modelTitleWeightRole: "fontWeight"

		autoSelectorChange: true

		refreshEnabled: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFlipable {
			id: flipable
			width: list.delegateHeight
			height: list.delegateHeight

			anchors.verticalCenter: parent.verticalCenter

			mouseArea.enabled: true

			frontIcon: CosStyle.iconInvisible
			backIcon: CosStyle.iconVisible
			color: model && model.active ? CosStyle.colorOKLighter : CosStyle.colorPrimaryDark
			flipped: model && model.active

			mouseArea.onClicked: {
				teacherGroups.send("groupMapActivate", {
									   id: teacherGroups.selectedGroupId,
									   active: !model.active,
									   uuid: model.uuid
								   })
			}
		}

		rightComponent: QFontImage {
			width: visible ? list.delegateHeight : 0
			height: width*0.8
			size: Math.min(height*0.8, 32)

			icon: CosStyle.iconDownloadCloud

			visible: model && !model.downloaded

			color: CosStyle.colorAccent
		}


		footer: QToolButtonFooter {
			width: list.width
			action: actionMapAdd
		}


		onRefreshRequest: teacherGroups.send("groupGet", { id: teacherGroups.selectedGroupId })

		onClicked: {
			var o = list.model.get(index)
			if (!o.downloaded)
				actionDownload.trigger()
			else
				JS.createPage("TeacherGroupMapView", {
								  teacherGroups: teacherGroups,
								  defaultTitle: o.name,
								  mapUuid: o.uuid
							  })
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

			MenuItem { action: actionActivate }
			MenuItem { action: actionDeactivate }
			MenuItem { action: actionRemove }
			MenuItem { action: actionDownload }
		}


		//onKeyInsertPressed: actionMapNew.trigger()
		onKeyF2Pressed: {
			var o = teacherGroups.modelMapList.get(sourceIndex)
			if (o.active)
				actionDeactivate.trigger()
			else
				actionActivate.trigger()
		}

		onKeyDeletePressed: actionRemove.trigger()
		//onKeyF4Pressed: actionObjectiveNew.trigger()
	}


	Action {
		id: actionActivate
		text: qsTr("Aktiválás")
		icon.source: CosStyle.iconVisible
		enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherGroups.modelMapList.selectedCount

			if (more > 0)
				teacherGroups.send("groupMapActivate", {
									   id: teacherGroups.selectedGroupId,
									   active: true,
									   list: teacherGroups.modelMapList.getSelectedData("uuid")
								   })
			else
				teacherGroups.send("groupMapActivate", {
									   id: teacherGroups.selectedGroupId,
									   active: true,
									   uuid: o.uuid
								   })
		}
	}


	Action {
		id: actionDeactivate
		text: qsTr("Inaktiválás")
		icon.source: CosStyle.iconInvisible
		enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherGroups.modelMapList.selectedCount

			if (more > 0)
				teacherGroups.send("groupMapActivate", {
									   id: teacherGroups.selectedGroupId,
									   active: false,
									   list: teacherGroups.modelMapList.getSelectedData("uuid")
								   })
			else
				teacherGroups.send("groupMapActivate", {
									   id: teacherGroups.selectedGroupId,
									   active: false,
									   uuid: o.uuid
								   })
		}
	}

	Action {
		id: actionRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconRemove
		enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherGroups.modelMapList.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Pálya eltávolítása"),
												text: qsTr("Biztosan eltávolítod a kijelölt %1 pályát?").arg(more)
											})
				dd.accepted.connect(function () {
					teacherGroups.send("groupMapRemove", {
										   id: teacherGroups.selectedGroupId,
										   list: teacherGroups.modelMapList.getSelectedData("uuid") })
				})
				dd.open()
			} else {
				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Pálya eltávolítása"),
											   text: qsTr("Biztosan eltávolítod a pályát?\n%1").arg(o.name)
										   })
				d.accepted.connect(function () {
					teacherGroups.send("groupMapRemove", {
										   id: teacherGroups.selectedGroupId,
										   uuid: o.uuid
									   })
				})
				d.open()
			}
		}
	}



	Action {
		id: actionDownload
		text: qsTr("Letöltés")
		icon.source: CosStyle.iconDownload
		enabled: !teacherGroups.isBusy && (list.currentIndex !== -1 || teacherGroups.modelMapList.selectedCount)
		onTriggered: {
			var o = list.model.get(list.currentIndex)

			var more = teacherGroups.modelMapList.selectedCount

			if (more > 0)
				teacherGroups.mapDownload({ list: teacherGroups.modelMapList.getSelectedData("uuid") })
			else
				teacherGroups.mapDownload({ uuid: o.uuid })
		}
	}

}



