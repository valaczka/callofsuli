import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: pageEditor

	property int mapId: -1
	property string mapName: ""
	property alias map: map
	property bool mapBinaryFormat: true
	property bool isPageBusy: false

	property bool _isFirstRun: true
	property bool _isMapLoaded: false

	signal pagePopulated()

	signal campaignSelected(int id)
	signal missionSelected(int id, int parentCampaignId)
	signal summarySelected(int id, int parentCampaignId)
	signal storageSelected(int id, int parentMId, int parentSId)
	signal objectiveSelected(int id, int parentMId, int parentSId)
	signal introSelected(int id, int parentId, int parentType)

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

		backButtonIcon: panelLayout.noDrawer ? CosStyle.iconBack : CosStyle.iconDrawer
		backButton.visible: true
		backButton.onClicked: {
			if (panelLayout.noDrawer)
				mainStack.back()
			else
				panelLayout.drawerToggle()
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
				dbActivity: map
				visible: _isMapLoaded
			}

			QToolButton {
				action: actionSave
				display: AbstractButton.IconOnly
			}

			QMenuButton {
				visible: _isMapLoaded

				MenuItem {
					text: qsTr("Mentés")
					onClicked:  {
						map.save(mapId, mapBinaryFormat)
					}
				}
				MenuItem {
					text: qsTr("Exportálás")
					onClicked:  {
						fileDialogSave.open()
					}
				}
			}
		}
	}

	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
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

	QPanelLayout {
		id: panelLayout
		anchors.fill: parent

		visible: _isMapLoaded
	}

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


	Keys.onPressed: if (_isMapLoaded) {
						if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier) && map.canUndo > -1)
							map.undo(map.canUndo-1)
					}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		panelLayout.drawerReset()

		if (_isFirstRun) {
			pagePopulated()
			_isFirstRun = false
		}
	}

	StackView.onDeactivated: {
		/* UNLOAD */
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


	function loadSettings() {
		toolbar.title = qsTr("Beállítások")
		panelLayout.panels = [
					{ url: "MapEditorSettings.qml", params: { map: map }, fillWidth: true }
				]
	}


	function loadCampaigns() {
		toolbar.title = qsTr("Hadjáratok")
		panelLayout.panels = [
					{ url: "MapEditorCampaignList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorCampaign.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadMissions() {
		toolbar.title = qsTr("Küldetések")
		panelLayout.panels = [
					{ url: "MapEditorMissionList.qml", params: { map: map }, fillWidth: true }
				]
	}


	function loadMission(mId, isSum) {
		toolbar.title = isSum ? qsTr("Összegzés") : qsTr("Küldetés")
		panelLayout.panels = [
					{ url: "MapEditorMission.qml", params: { map: map, missionId: mId, isSummary: isSum }, fillWidth: false },
					{ url: "MapEditorObjective.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadStorages() {
		toolbar.title = qsTr("Célpontok")
		panelLayout.panels = [
					{ url: "MapEditorStorageList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorObjective.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadIntros() {
		toolbar.title = qsTr("Introk/Outrok")
		panelLayout.panels = [
					{ url: "MapEditorIntroList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorIntro.qml", params: { map: map }, fillWidth: true }
				]

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


	function setBusy(busy) {
		pageEditor.isPageBusy=busy
	}

	function stackBack() {
		if (mainStack.depth > pageEditor.StackView.index+1) {
			if (!mainStack.get(pageEditor.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageEditor.StackView.index+1) {
					mainStack.pop(pageEditor)
				}
			}
			return true
		}

		if (panelLayout.layoutBack()) {
			return true
		}

		if (toolbar.title !== qsTr("Hadjáratok")) {
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

		panelLayout.drawer.close()

		return false
	}
}
