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

	title: qsTr("Feladatelosztás")
	icon: "image://font/Academic/\uf15a"

	property int mapId: -1

	contextMenuFunc: function (m) {
		m.addAction(actionChapterAdd)
		m.addAction(actionChapterRemove)
		m.addAction(actionRemove)
	}

	property VariantMapModel modelBlock: mapEditor.newModel(["block"])
	property VariantMapModel modelChapter: mapEditor.newModel(["chapter", "name"])

	Connections {
		target: mapEditor

		function onBlockChapterMapSelected(id) {
			mapId = id
		}

		function onBlockChapterMapLoaded(data) {
			modelBlock.clear()
			modelChapter.clear()

			if (!Object.keys(data).length) {
				mapId = -1
				return
			}

			if (data.id !== mapId) {
				mapId = -1
				return
			}

			modelBlock.setVariantList(data.blocks, "block")
			modelChapter.setVariantList(data.chapters, "chapter")

			blockCollapsible.collapsed = !data.blocks.length

		}


		function onBlockChapterMapRemoved() {
			mapEditor.run("blockChapterMapLoad", {id: mapId})
		}


		function onBlockChapterMapBlockListLoaded(id, list) {
			if (!list.length) {
				cosClient.sendMessageWarning(qsTr("Csatatér hozzáadása"), qsTr("Nincs több hozzáadható csatatér!"))
				return
			}

			var model = mapEditor.newModel(["details", "block"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["details", "block"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Csatatér hozzáadása"),
										   selectorSet: true,
										   sourceModel: model
									   })

			model.setVariantList(list, "block")

			d.accepted.connect(function(data) {
				mapEditor.run("blockChapterMapBlockAdd", {rowid: mapEditor.selectedLevelRowID, id: id, list: model.getSelectedData("block") })
			})
			d.open()
		}



		function onBlockChapterMapChapterListLoaded(id, list) {
			if (!list.length) {
				cosClient.sendMessageWarning(qsTr("Szakasz hozzáadása"), qsTr("Nincs több hozzáadható szakasz!"))
				return
			}

			var model = mapEditor.newModel(["name", "id"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["name", "id"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Szakasz hozzáadása"),
										   selectorSet: true,
										   sourceModel: model
									   })

			model.setVariantList(list, "id")

			d.accepted.connect(function(data) {
				mapEditor.run("blockChapterMapChapterAdd", {rowid: mapEditor.selectedLevelRowID, id: id, list: model.getSelectedData("id") })
			})
			d.open()
		}
	}

	QLabel {
		id: noLabel
		opacity: mapId == -1
		visible: opacity != 0

		anchors.centerIn: parent

		text: qsTr("Válassz feladatelosztást")

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}


	QAccordion {
		anchors.fill: parent

		opacity: mapId != -1
		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }

		QCollapsible {
			id: blockCollapsible
			title: qsTr("Csataterek")

			QVariantMapProxyView {
				id: blockList

				model: SortFilterProxyModel {
					sourceModel: modelBlock

					sorters: [
						StringSorter { roleName: "block" }
					]

					proxyRoles: [
						ExpressionRole {
							name: "fullname"
							expression: qsTr("%1. csatatér").arg(model.block)
						}
					]
				}

				autoSelectorChange: true

				delegateHeight: CosStyle.halfLineHeight

				modelTitleRole: "fullname"

				width: parent.width-blockCol.width
			}

			Column {
				id: blockCol
				anchors.left: blockList.right

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionBlockAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionBlockRemove
				}

			}

		}



		QCollapsible {
			title: qsTr("Szakaszok")

			QToolButtonBig {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionChapterAdd
				visible: !modelChapter.count
				color: CosStyle.colorOK
			}

			QVariantMapProxyView {
				id: chapterList

				visible: modelChapter.count

				model: SortFilterProxyModel {
					sourceModel: modelChapter

					sorters: [
						StringSorter { roleName: "name" }
					]
				}

				autoSelectorChange: true

				colorTitle: CosStyle.colorPrimaryLight
				fontWeightTitle: Font.Medium

				modelTitleRole: "name"

				width: parent.width-chapterCol.width
			}

			Column {
				id: chapterCol
				anchors.left: chapterList.right

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionChapterAdd
				}

				QToolButton {
					display: AbstractButton.IconOnly
					action: actionChapterRemove
				}

			}

		}


	}



	Action {
		id: actionRemove
		text: qsTr("Elosztás törlése")
		icon.source: CosStyle.iconDelete
		enabled: mapEditor.modelBlockChapterMapList.count > 1 && mapId > 0

		onTriggered: mapEditor.run("blockChapterMapRemove", {
									   rowid: mapEditor.selectedLevelRowID,
									   id: mapId
								   })
	}


	Action {
		id: actionBlockAdd
		text: qsTr("Csatatér hozzáadása")
		icon.source: CosStyle.iconLockAdd
		enabled: mapId > 0

		onTriggered: {
			mapEditor.run("blockChapterMapBlockGetList", {id: mapId})
		}
	}


	Action {
		id: actionBlockRemove
		text: qsTr("Csatatér eltávolítása")
		icon.source: CosStyle.iconDelete
		enabled: mapId > 0 && (blockList.currentIndex !== -1 || modelBlock.selectedCount)
		onTriggered: {
			var o = blockList.model.get(blockList.currentIndex)
			var more = modelBlock.selectedCount

			if (more > 0)
				mapEditor.run("blockChapterMapBlockRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  id: mapId,
								  list: modelBlock.getSelectedData("block")
							  })
			else
				mapEditor.run("blockChapterMapBlockRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  id: mapId,
								  block: o.block
							  })
		}
	}



	Action {
		id: actionChapterAdd
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconLockAdd
		enabled: mapId > 0

		onTriggered: {
			mapEditor.run("blockChapterMapChapterGetList", {id: mapId})
		}
	}



	Action {
		id: actionChapterRemove
		text: qsTr("Eltávolítás")
		icon.source: CosStyle.iconDelete
		enabled: mapId > 0 && (chapterList.currentIndex !== -1 || modelChapter.selectedCount)
		onTriggered: {
			var o = chapterList.model.get(chapterList.currentIndex)
			var more = modelChapter.selectedCount

			if (more > 0)
				mapEditor.run("blockChapterMapChapterRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  id: mapId,
								  list: modelChapter.getSelectedData("chapter")
							  })
			else
				mapEditor.run("blockChapterMapChapterRemove", {
								  rowid: mapEditor.selectedLevelRowID,
								  id: mapId,
								  chapter: o.chapter
							  })
		}
	}


	Component.onCompleted: {
		mapEditor.levelComponentsCompleted++
	}

}



