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


	signal campaignSelected(int id)

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

		rightLoader.sourceComponent: Row {
			QToolBusyIndicator { running: false }
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

		drawer.y: toolbar.height

		leftPanel: PageMapEditorRoot {
			anchors.fill: parent
		}
	}

	FileDialog {
		id: fileDialogSave
		title: qsTr("Exportálás")
		selectExisting: false
		sidebarVisible: false

		onAccepted: map.saveToFile(fileUrl)
	}


	Component {
		id: dlgModify

		QDialogYesNo {
			id: dlgYesNo

			title: qsTr("Biztosan eldobod a változtatásokat?")

			onDlgAccept: {
				map.mapModified = false
				mainStack.back()
			}

		}
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		panelLayout.reset(true)
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}



	function loadSettings() {
		panelLayout.components = [
					{ url: "PageMapEditorSettings.qml", params: { map: map } }
				]
	}


	function loadCampaigns() {
		panelLayout.components = [
					{ url: "PageMapEditorCampaignList.qml", params: { map: map } },
					{ url: "PageMapEditorCampaign.qml", params: { map: map } }
				]
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

		if (map.mapModified) {
			var d = JS.dialogCreate(dlgModify)
			d.open()
			return true
		}

		panelLayout.drawer.close()

		return false
	}
}
