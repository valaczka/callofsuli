import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	layoutFillWidth: true

	title: qsTr("Felszerelés")
	icon: CosStyle.iconUsers

	/*contextMenuFunc: function (m) {
		m.addAction(actionCampaignNew)
		m.addAction(actionMissionNew)
	}*/

	property VariantMapModel modelInventoryList: mapEditor.newModel(["rowid", "block", "module", "count"])


	Connections {
		target: mapEditor


		function onLevelLoaded(data) {
			if (!Object.keys(data).length) {
				return
			}

			if (data.rowid !== mapEditor.selectedLevelRowID)
				return

			modelInventoryList.setVariantList(data.inventory, "rowid")
		}
	}


	QAccordion {
		QCollapsible {
			title: qsTr("Inventory")

			QVariantMapProxyView {
				id: inventoryView

				model: SortFilterProxyModel {
					id: inventoryModel

					sourceModel: modelInventoryList

					sorters: [
						StringSorter { roleName: "module" }
					]

					/*proxyRoles: [
						ExpressionRole {
							name: "fullname"
							expression: model.level ? model.name+" ("+model.level+". szint)" : model.name
						}
					]*/
				}

				autoSelectorChange: true

				//delegateHeight: CosStyle.halfLineHeight

				modelTitleRole: "module"
				modelSubtitleRole: "count"

				width: parent.width-missionLocksCol.width
			}

			Column {
				id: missionLocksCol
				anchors.left: inventoryView.right

				QToolButton {
					display: AbstractButton.IconOnly
					//action: actionMissionLockAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					//action: actionMissionLockRemove
				}

				QToolButton {
					display: AbstractButton.IconOnly
					//action: actionMissionLockModify
				}
			}

		}


	}


	/*
	Action {
		id: actionMissionLockAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconLockAdd

		onTriggered: mapEditor.run("missionLockGetList", {uuid: missionUuid})
	}

*/


	Component.onCompleted: {
		mapEditor.levelComponentsCompleted++
	}

}



