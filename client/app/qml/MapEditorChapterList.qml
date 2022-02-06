import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QTabContainer {
	id: control

	title: qsTr("Feladatok")
	icon: CosStyle.iconComputer

	readonly property int contextAction: (MapEditorAction.ActionTypeChapterList | MapEditorAction.ActionTypeChapter)
	property int actionContextType: -1
	property var actionContextId: null

	signal forceChapterOpen(int chapterid)

	QObjectListDelegateView {
		id: chapterList
		anchors.fill: parent

		selectorSet: mapEditor.editor.chapters.selectedCount

		model: SortFilterProxyModel {
			sourceModel: mapEditor.editor.chapters

			sorters: RoleSorter {
				roleName: "id"
			}
		}

		delegate: MapEditorChapter {
			id: chapterItem
			required property int index
			collapsed: true
			level: -1
			selectorSet: chapterList.selectorSet
			onLongClicked: chapterList.onDelegateLongClicked(index)
			onSelectToggled: chapterList.onDelegateClicked(index, withShift)
			self: chapterList.modelObject(index)

			onSelfChanged: if (!self) {
							   delete chapterItem
						   }

			onChapterRemove: {
				if (mapEditor.editor.chapters.selectedCount > 0) {
					mapEditor.chapterRemoveList(mapEditor.editor.chapters.getSelected())
				} else {
					mapEditor.chapterRemove(self)
				}
			}

			Connections {
				target: control
				function onForceChapterOpen(chid) {
					if (!chapterItem.self)
						return

					if (chid === chapterItem.self.id)
						chapterItem.collapsed = false
					else
						chapterItem.collapsed = true
				}
			}
		}

		header: QTabHeader {
			tabContainer: control
			isPlaceholder: true
		}


		footer: Column {
			width: chapterList.width

			QToolButtonFooter {
				anchors.horizontalCenter: parent.horizontalCenter
				action: actionChapterNew
			}

			QToolButtonFooter {
				anchors.horizontalCenter: parent.horizontalCenter
				icon.source: CosStyle.iconAdd
				text: qsTr("Importálás")

				onClicked: {
					var d = JS.dialogCreateQml("File", {
												   isSave: false,
												   folder: cosClient.getSetting("mapFolder", ""),
											   })
					d.item.filters = ["*.xlsx", "*.xls"]

					d.accepted.connect(function(data){
						mapEditor.chapterImport({filename: data})
						cosClient.setSetting("mapFolder", d.item.modelFolder)
					})

					d.open()
				}
			}
		}
	}


	Action {
		id: actionChapterNew
		icon.source: CosStyle.iconAdd
		text: qsTr("Új szakasz")

		onTriggered: {
			var d = JS.dialogCreateQml("TextField", {
										   title: qsTr("Új szakasz"),
										   text: qsTr("Az új szakasz neve")
									   })

			d.accepted.connect(function(data) {
				if (data.length)
					mapEditor.chapterAdd({name: data})
			})
			d.open()
		}
	}

	onPopulated: {
		chapterList.forceActiveFocus()

		loadContextId(actionContextType, actionContextId)
		actionContextType = -1
		actionContextId = null
	}


	function loadContextId(type, id) {
		if (type === MapEditorAction.ActionTypeChapterList)
			forceChapterOpen(-1)
		else if (type === MapEditorAction.ActionTypeChapter)
			forceChapterOpen(id)
	}


	backCallbackFunction: function () {
		if (mapEditor.chapterModelUnselectObjectives(mapEditor.editor.chapters))
			return true

		if (mapEditor.editor.chapters.selectedCount) {
			mapEditor.editor.chapters.unselectAll()
			return true
		}

		return false
	}
}

