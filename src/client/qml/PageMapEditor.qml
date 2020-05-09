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
	signal chapterSelected(int id, int parentMId, int parentSId)
	signal introSelected(int id, int parentId, int parentType)

	Map {
		id: map

		client: cosClient
		mapType: Map.MapEditor

		onCanUndoChanged: if (canUndo === -1)
							  map.mapModified=false

		onMapLoadingProgress: {
			console.debug ("progress", progress)
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
				onClicked: pageEditor.loadChapters()
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
				MenuItem {
					text: qsTr("Pálya átnevezés")
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


	Keys.onPressed: if (_isMapLoaded) {
						if (event.key === Qt.Key_S && (event.modifiers & Qt.ControlModifier))
							map.save(mapId, mapBinaryFormat)
						else if (event.key === Qt.Key_Z && (event.modifiers & Qt.ControlModifier) && map.canUndo > -1)
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
		onTriggered: loadChapters()
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
					{ url: "MapEditorCampaign.qml", params: { map: map }, fillWidth: true },
					{ url: "MapEditorMission.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadMissions() {
		toolbar.title = qsTr("Küldetések")
		panelLayout.panels = [
					{ url: "MapEditorMissionList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorMission.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadChapters() {
		toolbar.title = qsTr("Célpontok")
		panelLayout.panels = [
					{ url: "MapEditorChapterList.qml", params: { map: map }, fillWidth: true }
				]
	}

	function loadIntros() {
		toolbar.title = qsTr("Introk/Outrok")
		panelLayout.panels = [
					{ url: "MapEditorIntroList.qml", params: { map: map }, fillWidth: false },
					{ url: "MapEditorIntro.qml", params: { map: map }, fillWidth: true }
				]

	}


	onChapterSelected: {
		var o = JS.createPage("MapChapterEditor", {
								  map: map,
								  chapterId: id,
								  parentMissionId: parentMId,
								  parentSummaryId: parentSId
							  }, pageEditor)
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
		if (panelLayout.layoutBack()) {
			return true
		}

		if (mainStack.depth > pageEditor.StackView.index+1) {
			if (!mainStack.get(pageEditor.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageEditor.StackView.index+1) {
					mainStack.pop(pageEditor)
				}
			}
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
