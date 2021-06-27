import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QCollapsible {
	id: control
	title: qsTr("Zárolások")

	titleColor: CosStyle.colorAccentLight
	backgroundColor: CosStyle.colorErrorDark
	contentBackgroundColor: JS.setColorAlpha(CosStyle.colorErrorDark, 0.8)

	QVariantMapProxyView {
		id: list

		width: parent.width

		model: SortFilterProxyModel {
			sourceModel: mapEditor.modelLockList

			sorters: StringSorter {
				roleName: "name"
			}
		}

		modelTitleRole: "name"
		colorTitle: CosStyle.colorAccentLighter

		autoSelectorChange: false
		refreshEnabled: false

		delegateHeight: CosStyle.baseHeight


		leftComponent: Item {
			width: list.delegateHeight
			height: width

			QBadge {
				anchors.centerIn: parent
				text: model && model.level ? model.level : ""
				color: CosStyle.colorErrorDark
			}
		}

		rightComponent: QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			icon.source: CosStyle.iconDelete
			color: CosStyle.colorAccentLight
			onClicked: {
				mapEditor.missionLockRemove({lock: model.lock})
			}
		}


		footer: QToolButtonFooter {
			width: list.width
			color: CosStyle.colorAccent
			text: qsTr("Új zárolás")
			icon.source: CosStyle.iconAdd
			onClicked: {
				mapEditor.missionLockGetList({})
			}
		}


		onClicked: {
			var o = list.model.get(index)

			mapEditor.missionLockGetList({lock: o.lock})
		}

	}


	Connections {
		target: mapEditor

		function onMissionLockListReady(data) {
			if (data.mission !== mapEditor.currentMission)
				return

			var isUpdate = (data.lock && data.lock.length)


			mapEditor.modelDialogMissionList.replaceList(data.levels)

			var d = JS.dialogCreateQml("MissionList", {
										   icon: CosStyle.iconLockAdd,
										   title: isUpdate ? qsTr("Zárolás szintjének módosítása") : qsTr("Zárolás hozzáadása"),
										   selectorSet: false,
										   sourceModel: mapEditor.modelDialogMissionList
									   })


			d.accepted.connect(function(data) {
				if (data === -1)
					return

				var p = d.item.sourceModel.get(data)

				if (isUpdate) {
					mapEditor.missionLockModify({lock: p.uuid, level: p.level})
				} else {
					mapEditor.missionLockAdd({lock: p.uuid, level: p.level})
				}

			})
			d.open()

		}
	}

}
