import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageStart

	requiredPanelWidth: 900

	title: qsTr("Call of Suli")
	mainToolBar.backButton.visible: false

	Servers {
		id: servers
		client: cosClient

		property bool editing: false
		property int serverId: -1
	}

	property list<Component> normalComponents: [
		Component { ServerList {
				panelVisible: true
				layoutFillWidth: true
			} }
	]

	property list<Component> editComponents: [
		Component { ServerList {
				panelVisible: true
				layoutFillWidth: false
			} },
		Component { ServerEdit { panelVisible: true } }
	]

	panelComponents: servers.editing ? editComponents : normalComponents

	mainMenuFunc: function (m) {
		m.addAction(tmXtest)
		m.addAction(actionAbout)
		m.addAction(actionExit)
	}


	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		onTriggered: {
			JS.dialogMessageInfo("Call of Suli",
								 qsTr("Verzió: ")+Qt.application.version+
								 "\n© 2012-2020 Valaczka János Pál"
								 )
		}
	}

	Action {
		id: actionExit
		text: qsTr("Kilépés")
		onTriggered: mainWindow.close()
	}


	Action {
		id: test
		shortcut: "F1"
		onTriggered: {
			var o = JS.createPage("MapEditor", {})
			o.pagePopulated.connect(function() {
				o.map.loadFromFile("AAA.cosm")
				o.map.mapOriginalFile = "AAA.cosm"
				o.mapName = "AAA.cosm"
			})
		}
	}


	Action {
		id: tmXtest
		shortcut: "F2"
		text: "TMX"
		onTriggered: {
			var o = JS.createPage("TMXtest", {})
		}
	}


	Connections {
		target: cosClient

		onConnectionStateChanged: {
			if (connectionState === Client.Connected) {
				panels = []
				JS.createPage("MainMenu", {})
			} else if (connectionState === Client.Standby) {
				mainStack.pop(pageStart)
			}
		}
	}


	function windowClose() {
		return true
	}

	function pageStackBack() {
		if (servers.editing) {
			servers.editing = false
			return true
		}

		return false
	}
}
