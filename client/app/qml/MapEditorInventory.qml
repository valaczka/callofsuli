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

	required property GameMapEditorMissionLevel missionLevel

	property ListModel availableInventoryModel: ListModel {}

	backgroundColor: "darkblue"
	contentBackgroundColor: "midnightblue"


	QObjectListView {
		id: list

		width: parent.width

		model: SortFilterProxyModel {
			sourceModel: missionLevel.inventories

			sorters: StringSorter {
				roleName: "module"
			}

			proxyRoles: [
				ExpressionRole {
					name: "icon"
					expression: mapEditor.inventoryInfo(model.module).icon
				},
				ExpressionRole {
					name: "name"
					expression: mapEditor.inventoryInfo(model.module).name
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
					var i = list.modelObject(modelIndex)
					mapEditor.inventoryRemove(missionLevel, i)
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
				value: model ? model.count : 0
				editable: true
				width: 120

				onValueModified: {
					var i = list.modelObject(modelIndex)
					mapEditor.inventoryModify(i, {count: value})
				}
			}
		}


		footer: QToolButtonFooter {
			width: list.width
			text: qsTr("Felszerelés")
			icon.source: CosStyle.iconAdd
			color: "lightskyblue"
			onClicked: {
				var d = JS.dialogCreateQml("List", {
											   icon: CosStyle.iconLockAdd,
											   title: qsTr("Felszerelés hozzáadása"),
											   selectorSet: false,
											   modelTitleRole: "name",
											   modelImageRole: "icon",
											   delegateHeight: CosStyle.baseHeight,
											   model: availableInventoryModel
										   })


				d.accepted.connect(function(data) {
					if (!data)
						return

					mapEditor.inventoryAdd(missionLevel, { module: data.module })
				})
				d.open()
			}
		}


		onClicked: {
			var o = list.modelObject(index)
			var b = o.block > 0 ? o.block : ""
			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Csatatér"),
										   text: qsTr("Csatatér hozzárendelése (0 vagy semmi = bármelyik csatatér)"),
										   value: b,
										   inputMethodHints: Qt.ImhDigitsOnly })

			d.accepted.connect(function(data) {
				if (data.length)
					mapEditor.inventoryModify(o, { block: Number(data) })
				else
					mapEditor.inventoryModify(o, { block: 0 })
			})
			d.open()
		}

	}


	Component.onCompleted: {
		var l = mapEditor.availableInventories

		for (var i=0; i<l.length; i++) {
			availableInventoryModel.append(l[i])
		}
	}
}
