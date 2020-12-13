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

	defaultTitle: qsTr("Pályaszerkesztő")

	property string loadFileName: cosClient.standardPath("ttt.dat")


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
			text: qsTr("Küldetések")
		}

		MenuItem {
			text: qsTr("Célpontok")
		}

	}


	activity: MapEditor {
		id: mapEditor

		property VariantMapModel modelCampaignList: newModel([
																 "type",
																 "cid",
																 "uuid",
																 "cname",
																 "mname",
																 "mandatory",
																 "locked"
															 ])


		onBackupUnavailable: {
			if (page.loadFileName.length) {
				mapEditor.loadFromFile({filename: page.loadFileName})
			} else {
				mapEditor.createNew({name: qsTr("--- ez egy új map---")})
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
			actionCampaigns.trigger()
		}


		function terrainList() {
			var model = mapEditor.newModel(["details", "name"])

			var d = JS.dialogCreateQml("List", {
										   roles: ["details", "name"],
										   icon: CosStyle.iconLockAdd,
										   title: qsTr("Terep kiválasztása"),
										   selectorSet: false,
										   sourceModel: model
									   })

			console.debug(cosClient.terrainList())

			model.setVariantList(cosClient.terrainList(), "name")

			return d
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
		enabled: mapEditor.modified
		shortcut: "Ctrl+S"
		onTriggered: {
			mapEditor.saveToFile({filename: page.loadFileName})
		}
	}



	Action {
		id: actionCampaigns

		text: qsTr("Küldetések")
		icon.source: CosStyle.iconAdjust
		onTriggered: {
			page.panelComponents = page.cmpCampaigns
		}
	}


	onPageActivated: {
		mapEditor.checkBackup()
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

		mapEditor.removeDatabase()
		return false
	}


	function stackBack() {
		if (mapEditor.modified) {
			var d = JS.dialogCreateQml("YesNo", {text: qsTr("Biztosan eldobod a módosításokat?")})
			d.accepted.connect(function() {
				mapEditor.modified = false
				mainStack.back()
			})
			d.open()
			return true
		}

		mapEditor.removeDatabase()
		return false
	}

}
