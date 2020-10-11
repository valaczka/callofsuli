import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.14
import QtMultimedia 5.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageStart

	requiredPanelWidth: 900

	property bool _isFirst: true

	signal serverEdit(var id)
	signal serverConnect(var id)
	signal serverCreate()
	signal unloadEditor()

	Servers {
		id: servers
		client: cosClient
	}

	property bool isFirstRun: true

	title: qsTr("Call of Suli")
	mainToolBar.backButton.visible: false

	pageContextMenu: QMenu {
		MenuItem { action: actionAbout }
		MenuItem { action: actionExit }
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
				onePanel()
			}
		}
	}

	onServerEdit:  {
		if (panels.length < 2)
			panels = [
						{ url: "ServerList.qml", params: { servers: servers }, fillWidth: false},
						{ url: "ServerEdit.qml", params: { servers: servers, serverId: id }, fillWidth: true}
					]

		swipeToPage(1)
	}

	onServerCreate:  {
		if (panels.length < 2)
			panels = [
						{ url: "ServerList.qml", params: { servers: servers }, fillWidth: false},
						{ url: "ServerEdit.qml", params: { servers: servers, serverId: -1 }, fillWidth: true}
					]
		swipeToPage(1)
	}

	onUnloadEditor: onePanel()


	onServerConnect: {
		if (panels.length > 1)
			serverEdit(id)
		else
			servers.serverConnect(id)
	}

	onPageActivated: {
		if (_isFirst) {
			var autoConnectId = servers.serverListReload()
			_isFirst = false

			if (autoConnectId !== -1)
				servers.serverConnect(autoConnectId)
		}

		onePanel()
	}

	function onePanel() {
		panels = [
					{ url: "ServerList.qml", params: { servers: servers }, fillWidth: true}
				]
	}

	function windowClose() {
		return true
	}

	function pageStackBack() {
		if (panels.length>1) {
			onePanel()
			return true
		}
		return false
	}
}
