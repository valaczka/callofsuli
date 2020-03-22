import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: control
	title: qsTr("Felszerelés")

	required property int level

	QVariantMapProxyView {
		id: list

		width: parent.width

		model: SortFilterProxyModel {
			sourceModel: mapEditor.modelInventoryList

			filters: ValueFilter {
				roleName: "level"
				value: control.level
			}

			sorters: RoleSorter {
				roleName: "rid"
			}

			proxyRoles: [
				ExpressionRole {
					name: "icon"
					expression: cosClient.inventoryInfo(model.module).icon
				},
				ExpressionRole {
					name: "name"
					expression: cosClient.inventoryInfo(model.module).name
				}
			]
		}

		modelTitleRole: "name"
		colorTitle: CosStyle.colorAccentLighter
		pixelSizeTitle: CosStyle.pixelSize*0.9


		autoSelectorChange: false
		refreshEnabled: false

		delegateHeight: CosStyle.twoLineHeight*1.5


		leftComponent: Image {
			width: visible ? list.delegateHeight : 0
			height: width*0.6
			fillMode: Image.PreserveAspectFit

			source: model ? model.icon : ""

			visible: model && model.icon.length
		}

		rightComponent: Row {
			spacing: 0

			QBadge {
				anchors.verticalCenter: parent.verticalCenter
				text: model ? model.block : ""
				color: CosStyle.colorPrimaryDarker
				visible: model && model.block > 0
			}

			QToolButton {
				anchors.verticalCenter: parent.verticalCenter
				icon.source: CosStyle.iconDelete
				color: CosStyle.colorErrorLighter
				onClicked: {
					mapEditor.inventoryRemove({level: control.level, rid: model.rid})
				}
			}
		}

		contentComponent: Item {
			height: sp.height
			QSpinBox {
				id: sp
				from: 1
				to: 99
				stepSize: 1
				value: model ? model.icount : 0
				editable: true
				width: 120

				onValueModified: {
					mapEditor.inventoryModify({level: control.level, rid: model.rid, count: value})
				}
			}
		}


		footer: QToolButtonFooter {
			width: list.width
			//height: Math.max(implicitHeight, CosStyle.twoLineHeight)
			text: qsTr("Új felszerelés")
			icon.source: CosStyle.iconAdd
			onClicked: {
				var d = JS.dialogCreateQml("List", {
											   roles: ["name", "icon"],
											   icon: CosStyle.iconLockAdd,
											   title: qsTr("Felszerelés hozzáadása"),
											   selectorSet: false,
											   modelImageRole: "icon",
											   delegateHeight: CosStyle.twoLineHeight,
											   sourceModel: mapEditor.modelInventoryModules
										   })


				d.accepted.connect(function(data) {
					if (data === -1)
						return

					var p = d.item.sourceModel.get(data)
					mapEditor.inventoryAdd({level: container.level, module: p.module})
					//mapEditor.missionLevelModify({level: container.level, terrain: p.name})

				})
				d.open()
			}
		}


		onClicked: {
			var o = list.model.get(index)
			var b = o.block > 0 ? o.block : ""
			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Csatatér"),
										   text: qsTr("Csatatér hozzárendelése (0 vagy semmi = bármelyik csatatér)"),
										   value: b,
										   inputMethodHints: Qt.ImhDigitsOnly })

			d.accepted.connect(function(data) {
				if (data.length)
					mapEditor.inventoryModify({level: control.level, rid: o.rid, block: Number(data)})
				else
					mapEditor.inventoryModify({level: control.level, rid: o.rid, block: 0})
			})
			d.open()
		}

	}
}
