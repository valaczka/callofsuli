import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	property bool _isLoaded: false

	//requiredPanelWidth: 900

	title: qsTr("Pályaszerkesztő")

	property string loadFileName: ""

	mainToolBarComponent: Row {
		QUndoButton  {
			anchors.verticalCenter: parent.verticalCenter
			id: undoButton
			dbActivity: mapEditor.db
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
			text: qsTr("Küldetések")
		}

		MenuItem {
			text: qsTr("Célpontok")
		}

	}


	activity: MapEditor {
		id: mapEditor

		onBackupUnavailable: {
			if (page.loadFileName.length) {
				loadFromFile(page.loadFileName)
			} else {
				createNew(qsTr("---új map---"))
			}
		}

		onLoadStarted: {
			var d = JS.dialogCreateQml("Progress", { title: qsTr("Betöltés") })
			d.closePolicy = Popup.NoAutoClose

			d.rejected.connect(function(data) {
				console.debug("###########rejected")
				mapEditor.loadAbort()
			})

			mapEditor.loadProgressChanged.connect(d.item.setValue)
			mapEditor.loadFinished.connect(d.item.dlgClose)

			d.open()
		}

		onLoadFinished: {
			_isLoaded = true
			actionCampaigns.trigger()
		}
	}


	property list<Component> cmpCampaigns: [
		Component { MapEditorCampaignList {
				panelVisible: true
				layoutFillWidth: false
			} },
		Component { MapEditorCampaignEdit {
				panelVisible: true
			} }
	]



	Action {
		id: actionSave
		icon.source: CosStyle.iconSave
		enabled: false
		shortcut: "Ctrl+S"
	}


	Action {
		id: actionCampaigns

		text: qsTr("Hadjáratok")
		icon.source: CosStyle.iconAdjust
		onTriggered: {
			page.subtitle = text
			page.panelComponents = page.cmpCampaigns
			mapEditor.campaignListReload()
		}
	}

	/*Label {
		anchors.centerIn: parent
		text: "***** "+mapEditor.loadProgress
	}*/

	//mainMenuFunc: function(m) {}
	//contextMenuFunc: function(m) {}

	/*panelComponents: [
		Component { QPagePanel {
				panelVisible: true
				layoutFillWidth: true
			} }
	]*/

	onPageActivated: {
		mapEditor.checkBackup()
	}


	function windowClose() {
		mapEditor.removeDatabase()
		return true
	}

	function pageStackBack() {
		mapEditor.removeDatabase()
		return false
	}

}
