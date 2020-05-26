import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageEditor

	property int mapId: -1
	property string mapName: ""
	property bool mapBinaryFormat: true
	property bool isPageBusy: false

	property bool _isFirstRun: true
	property bool _isMapLoaded: false

	property int _backPool: 0

	signal pagePopulated()

	signal campaignSelected(int id)
	signal missionSelected(int id, int parentCampaignId)
	signal summarySelected(int id, int parentCampaignId)
	signal storageSelected(int id, int parentMId, int parentSId)
	signal objectiveSelected(int id, int parentMId, int parentSId)
	signal introSelected(int id, int parentId, int parentType)

	property alias map: map


	MapEditor {
		id: map
		client: cosClient

		onCanUndoChanged: if (canUndo === -1)
							  map.mapModified=false

		onMapLoadingProgress: {
			progressBar.value = progress
		}

		onMapLoaded: {
			_isMapLoaded = true
			loadCampaigns()
		}
	}


	header: QToolBar {
		id: toolbar

		title: mapName

		backButtonIcon: noDrawer ? CosStyle.iconBack : CosStyle.iconDrawer
		backButton.visible: true
		backButton.onClicked: {
			if (noDrawer)
				mainStack.back()
			else
				drawerToggle()
		}


		menuLoader.sourceComponent: QMenuButton {
			icon.source: CosStyle.iconDown

			visible: _isMapLoaded

			MenuItem {
				text: qsTr("Hadjáratok")
				onClicked: pageEditor.loadCampaigns()
			}

			MenuItem {
				text: qsTr("Küldetések")
				onClicked: pageEditor.loadMissions()
			}

			MenuItem {
				text: qsTr("Célpontok")
				onClicked: pageEditor.loadStorages()
			}


			MenuItem {
				text: qsTr("Introk/Outrok")
				onClicked: pageEditor.loadIntros()
			}
		}

		Row {
			QToolBusyIndicator { running: isPageBusy }

			QUndoButton  {
				id: undoButton
				dbActivity: map
				visible: _isMapLoaded
			}

			QToolButton {
				action: actionSave
				display: AbstractButton.IconOnly
			}

			QMenuButtonComposite {
				visible: _isMapLoaded

				baseItems: [
					MenuItem {
						text: qsTr("Mentés")
						onClicked:  {
							map.save(mapId, mapBinaryFormat)
						}
					},
					MenuItem {
						text: qsTr("Exportálás")
						onClicked:  {
							fileDialogSave.open()
						}
					}
				]
			}
		}
	}



	Item {
		id: loadingItem
		anchors.fill: parent

		visible: !_isMapLoaded

		ProgressBar {
			id: progressBar
			anchors.centerIn: parent
		}
	}

	panelsVisible: _isMapLoaded

	FileDialog {
		id: fileDialogSave
		title: qsTr("Exportálás")
		selectExisting: false
		sidebarVisible: false

		onAccepted: map.saveToFile(fileUrl)
	}


	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		text: qsTr("Mentés")
		shortcut: "Ctrl+S"
		enabled: _isMapLoaded && map.mapModified
		onTriggered: map.save(mapId, mapBinaryFormat)
	}


	Action {
		id: actionUndo
		shortcut: "Ctrl+Z"
		enabled: undoButton.enabled
		onTriggered: undoButton.undo()
	}


	StackView.onActivated: {
		if (_isFirstRun) {
			pagePopulated()
			_isFirstRun = false
		}
	}


	onMissionSelected: loadMission(id, false)
	onSummarySelected: loadMission(id, true)


	Action {
		shortcut: "F2"
		onTriggered: loadCampaigns()
	}

	Action {
		shortcut: "F3"
		onTriggered: loadMissions()
	}

	Action {
		shortcut: "F4"
		onTriggered: loadStorages()
	}

	Action {
		shortcut: "F5"
		onTriggered: loadIntros()
	}


	onCampaignSelected: swipeToPage(1)
	onObjectiveSelected: swipeToPage(1)
	onStorageSelected: swipeToPage(1)
	onIntroSelected: swipeToPage(1)


	function loadSettings() {
		_backPool = 0
		toolbar.title = qsTr("Beállítások")
		panels = [
					{ url: "MapEditorSettings.qml", params: { map: map }, fillWidth: true }
				]
	}


	function loadCampaigns() {
		_backPool = 0
		toolbar.title = qsTr("Hadjáratok")
		panels = [
					{ url: "MapEditorCampaignList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorCampaign.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadMissions() {
		_backPool = 1
		toolbar.title = qsTr("Küldetések")
		panels = [
					{ url: "MapEditorMissionList.qml", params: { map: map }, fillWidth: true }
				]
	}


	function loadMission(mId, isSum) {
		_backPool = 2
		toolbar.title = isSum ? qsTr("Összegzés") : qsTr("Küldetés")
		panels = [
					{ url: "MapEditorMission.qml", params: { map: map, missionId: mId, isSummary: isSum }, fillWidth: false },
					{ url: "MapEditorObjective.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadStorages() {
		_backPool = 2
		toolbar.title = qsTr("Célpontok")
		panels = [
					{ url: "MapEditorStorageList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorObjective.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadIntros() {
		_backPool = 2
		toolbar.title = qsTr("Introk/Outrok")
		panels = [
					{ url: "MapEditorIntroList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorIntro.qml", params: { map: map }, fillWidth: true }
				]

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


	function setBusy(busy) {
		pageEditor.isPageBusy=busy
	}

	function pageStackBack() {
		if (_backPool == 2) {
			loadMissions()
			return true
		} else if (_backPool == 1) {
			loadCampaigns()
			return true
		}

		if (map.mapModified) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan eldobod a változtatásokat?")})
			d.accepted.connect(function() {
				map.mapModified = false
				mainStack.back()
			})
			d.open()
			return true
		}

		return false
	}
}
