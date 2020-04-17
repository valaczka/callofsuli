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

	signal campaignSelected(int modelIndex, int id)
	signal missionSelected(int modelIndex, int id, int parentCampaignId)
	signal summarySelected(int modelIndex, int id, int parentCampaignId)
	signal chapterSelected(int modelIndex, int id, int parentMissionId, int parentSummaryId)
	signal introSelected(int modelIndex, int id, int parentId, int parentType)

	Map {
		id: map

		client: cosClient
		mapType: Map.MapEditor
	}


	header: QToolBar {
		id: toolbar

		title: mapName

		backButtonIcon: panelLayout.noDrawer ? "M\ue5c4" : "M\ue3c7"
		backButton.visible: true
		backButton.onClicked: {
			if (panelLayout.noDrawer)
				mainStack.back()
			else
				panelLayout.drawerToggle()
		}


		menuLoader.sourceComponent: QMenuButton {
			buttonLabel: "M\ue5c5"

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

		rightLoader.sourceComponent: Row {
			QToolBusyIndicator { running: isPageBusy }
			QMenuButton {
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

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPanelLayout {
		id: panelLayout
		anchors.fill: parent
	}

	FileDialog {
		id: fileDialogSave
		title: qsTr("Exportálás")
		selectExisting: false
		sidebarVisible: false

		onAccepted: map.saveToFile(fileUrl)
	}


	Keys.onPressed: {
		if (event.key === Qt.Key_S && (event.modifiers & Qt.ControlModifier))
			map.save(mapId, mapBinaryFormat)

	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		panelLayout.reset()
		loadCampaigns()
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function loadSettings() {
		toolbar.title = qsTr("Beállítások")
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "MapEditorSettings.qml", params: { map: map } }
					)
	}


	function loadCampaigns() {
		toolbar.title = qsTr("Hadjáratok")
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "MapEditorCampaignList.qml", params: { map: map } }
					)
	}

	function loadMissions() {
		toolbar.title = qsTr("Küldetések")
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "MapEditorMissionList.qml", params: { map: map } }
					)
	}

	function loadChapters() {
		toolbar.title = qsTr("Célpontok")
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "MapEditorChapterList.qml", params: { map: map } }
					)
	}

	function loadIntros() {
		toolbar.title = qsTr("Introk/Outrok")
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "MapEditorIntroList.qml", params: { map: map } }
					)
	}



	onCampaignSelected: panelLayout.loadPage(modelIndex, id,
											 "MapEditorCampaign.qml",
											 { map: map, campaignId: id })

	onMissionSelected: panelLayout.loadPage(modelIndex, id,
											"MapEditorMission.qml",
											{ map: map, missionId: id, isSummary: false, parentCampaignId: parentCampaignId })

	onSummarySelected: panelLayout.loadPage(modelIndex, id,
											"MapEditorMission.qml",
											{ map: map, missionId: id, isSummary: true, parentCampaignId: parentCampaignId })

	onChapterSelected: panelLayout.loadPage(modelIndex, id,
											"MapEditorChapter.qml",
											{ map: map, chapterId: id, parentMissionId: parentMissionId, parentSummaryId: parentSummaryId })

	onIntroSelected: panelLayout.loadPage(modelIndex, id,
										  "MapEditorIntro.qml",
										  { map: map, introId: id, parentId: parentId, parentType: parentType })

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
