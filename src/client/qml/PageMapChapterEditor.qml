import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: pageChapterEditor

	property MapEditor map: null
	property int chapterId: -1
	property int parentMissionId: -1
	property int parentSummaryId: -1

	property int storageId: -1
	property int objectiveId: -1

	signal reloadObjective()

	header: QToolBar {
		id: toolbar

		title: qsTr("Célpont")

		backButtonIcon: panelLayout.noDrawer ? CosStyle.iconBack : CosStyle.iconDrawer
		backButton.visible: true
		backButton.onClicked: {
			if (panelLayout.noDrawer)
				mainStack.back()
			else
				panelLayout.drawerToggle()
		}


		Row {
			QToolBusyIndicator { running: pageEditor.isPageBusy }

			QUndoButton  {
				dbActivity: map
			}
		}
	}

	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPanelLayout {
		id: panelLayout
		anchors.fill: parent

		panels: [
			{ url: "MapEditorChapter.qml", params: { map: map }, fillWidth: false },
			{ url: "MapEditorObjective.qml", params: { map: map }, fillWidth: true }
		]
	}

	Keys.onPressed: {
		if (event.key === Qt.Key_S && (event.modifiers & Qt.ControlModifier))
			map.save(mapId, mapBinaryFormat)
		else if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier) && map.canUndo > -1)
			map.undo(map.canUndo-1)
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		panelLayout.drawerReset()
	}


	function storageSelected(id) {
		objectiveId = -1
		storageId = id
		reloadObjective()
	}

	function objectiveSelected(id) {
		objectiveId = id
		storageId = -1
		reloadObjective()
	}





	function deleteChapter(txt) {
		if (chapterId == -1)
			return

		var d = JS.dialogCreateQml("YesNo")
		d.item.title = qsTr("Biztosan törlöd a célpontot?")
		d.item.text = txt
		d.accepted.connect(function () {
			map.undoLogBegin(qsTr("Célpont törlése"))
			if (map.chapterRemove(chapterId)) {
				map.undoLogEnd()
				mainStack.back()
			} else {
				map.undoLogEnd()
			}
		})
		d.open()
	}


	function closeDrawer() {
		panelLayout.drawer.close()
	}


	function windowClose() {
		if (map.mapModified) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan eldobod a változtatásokat?")})
			d.accepted.connect(function() {
				map.mapModified = false
				mainWindow.close()
			})
			d.open()
			return false
		}

		return true
	}



	function stackBack() {
		if (panelLayout.layoutBack()) {
			return true
		}

		if (mainStack.depth > pageChapterEditor.StackView.index+1) {
			if (!mainStack.get(pageChapterEditor.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageChapterEditor.StackView.index+1) {
					mainStack.pop(pageChapterEditor)
				}
			}
			return true
		}

		panelLayout.drawer.close()

		return false
	}
}
