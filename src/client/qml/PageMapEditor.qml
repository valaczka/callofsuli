import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	property bool _isLoaded: false
	property bool _hasBack: false

	defaultTitle: qsTr("Pályaszerkesztő")
	defaultSubTitle: mapEditor.mapName

	property alias mapName: mapEditor.mapName
	property alias fileName: mapEditor.fileName
	property alias database: mapEditor.database
	property alias databaseUuid: mapEditor.databaseUuid
	property alias databaseTable: mapEditor.databaseTable
	property alias mapEditor: mapEditor

	mainToolBarComponent: Row {
		QUndoButton  {
			anchors.verticalCenter: parent.verticalCenter
			id: undoButton
			activity: mapEditor
			visible: _isLoaded
		}
		QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			action: actionSave
			visible: _isLoaded
		}
	}


	mainToolBar.menuLoader.sourceComponent: QMenuButton {
		id: menuSelector
		icon.source: CosStyle.iconDown

		visible: _isLoaded

		MenuItem {
			action: actionCampaigns
		}

		MenuItem {
			action: actionChapters
		}
	}


	activity: MapEditor {
		id: mapEditor

		property VariantMapModel modelCampaignList: newModel([
																 "noorphan",
																 "type",
																 "cid",
																 "uuid",
																 "cname",
																 "mname",
																 "mandatory",
																 "locked"
															 ])

		property VariantMapModel modelChapterList: newModel([
																"type",
																"id",
																"uuid",
																"name",
																"module",
																"storage",
																"data"
															])

		property VariantMapModel modelBlockChapterMapList: mapEditor.newModel(["id", "enemies", "blocks"])

		property int selectedLevelRowID: -1
		property int levelComponentsCompleted: 0
		property string selectedMissionUUID: ""


		onLoadStarted: {
			var d = JS.dialogCreateQml("Progress", { title: qsTr("Betöltés") })
			d.closePolicy = Popup.NoAutoClose

			d.rejected.connect(function(data) {
				mapEditor.loadAbort()
			})

			mapEditor.loadProgressChanged.connect(d.item.setValue)
			mapEditor.loadFinished.connect(d.item.dlgClose)
			mapEditor.loadFailed.connect(d.item.dlgClose)

			d.onClosedAndDestroyed.connect(function() {
				mapEditor.loadProgressChanged.disconnect(d.item.setValue)
				mapEditor.loadFinished.disconnect(d.item.dlgClose)
				mapEditor.loadFailed.disconnect(d.item.dlgClose)
			})

			d.open()
		}


		/*onSaveStarted: {
			var d = JS.dialogCreateQml("Progress", { title: qsTr("Mentés") })
			d.closePolicy = Popup.NoAutoClose

			d.rejected.connect(function(data) {
				console.debug("###########rejected")
				mapEditor.loadAbort()
			})

			mapEditor.loadProgressChanged.connect(d.item.setValue)
			mapEditor.saveFinished.connect(d.item.dlgClose)

			d.open()
		}*/

		onSaveFinished: {
			mapEditor.modified = false
		}

		onLoadFinished: {
			_isLoaded = true
			actionChapters.trigger()
		}


		onLevelSelected: {
			selectedLevelRowID = rowid
			selectedMissionUUID = missionUuid
			levelComponentsCompleted = 0
			panelComponents = cmpLevel
			_hasBack = true
		}

		onLevelComponentsCompletedChanged: {
			if (levelComponentsCompleted == 3 && selectedLevelRowID != -1) {
				run("levelLoad", {rowid: selectedLevelRowID})
				levelComponentsCompleted = 0
			}
		}


		onChapterImportReady: {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("%1 tétel importálható?").arg(list.length)})
			d.accepted.connect(function() {
				run("objectiveImport", {list: list})
			})
			d.open()
			return true
		}
	}







	property list<Component> cmpCampaigns: [
		Component { MapEditorCampaignList {
			} },
		Component { MapEditorCampaignEdit {
			} }
	]


	property list<Component> cmpChapters: [
		Component { MapEditorChapterList {
			} },
		Component { MapEditorObjectiveEdit {
			} }
	]


	property list<Component> cmpLevel: [
		Component { MapEditorLevelGeneral {
			} },
		Component { MapEditorLevelChapters {
			} },
		Component { MapEditorLevelInventory {
			} }
	]


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		enabled: mapEditor.modified
		shortcut: "Ctrl+S"
		onTriggered: {
			mapEditor.saveDefault()
		}
	}



	Action {
		id: actionCampaigns

		text: qsTr("Játékmenet")
		icon.source: CosStyle.iconAdjust
		onTriggered: {
			panelComponents = cmpCampaigns
			_hasBack = false
		}
	}



	Action {
		id: actionChapters

		text: qsTr("Feladatok")
		icon.source: CosStyle.iconAdjust
		onTriggered: {
			panelComponents = cmpChapters
			mapEditor.selectedMissionUUID = ""
			_hasBack = true
		}
	}


	FileDialog {
		id: importDialog
		title: qsTr("Importálás")
		folder: shortcuts.home

		nameFilters: [ qsTr("Excel fájlok")+ " (*.xlsx)" ]

		property int chapterId: -1

		selectMultiple: false
		selectExisting: true
		selectFolder: false

		onAccepted: {
			mapEditor.run("chapterImport", {chapterid: chapterId, filename: importDialog.fileUrl})
		}
	}

	onPageActivated: {
		if (!_isLoaded) {
			mapEditor.loadDefault()
		}
	}




	function windowClose() {
		if (mapEditor.modified) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				mapEditor.modified = false
				mainWindow.close()
			})
			d.open()
			return true
		}

		mapEditor.removeImageProvider()

		return false
	}


	function pageStackBack() {
		if (_hasBack) {
			actionCampaigns.trigger()
			return true
		}

		if (mapEditor.modified) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				mapEditor.modified = false
				mainStack.back()
			})
			d.open()
			return true
		}

		mapEditor.removeImageProvider()

		return false
	}

}
