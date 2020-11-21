import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: control

	requiredPanelWidth: 900

	title: qsTr("Call of Suli")
	mainToolBar.backButton.visible: false

	property bool _firstRun: true

	activity: Servers {
		id: servers

		property bool editing: false
		property int serverKey: -1

		onResourcesChanged: loadLabel.setText(resources)

		onReadyResourcesChanged: if (readyResources) {
									 JS.createPage("MainMenu", {})
								 }

		Component.onCompleted: serverListReload()
	}

	property list<Component> fullComponents: [
		Component { ServerList {
				panelVisible: true
				layoutFillWidth: !servers.editing
			} },
		Component { ServerEdit { panelVisible: servers.editing} }
	]

	property list<Component> listComponents: [
		Component { ServerList {
				panelVisible: true
				layoutFillWidth: true
			} }
	]

	swipeMode: width < 900

	panelComponents: if (swipeMode)
						 servers.editing ? fullComponents : listComponents
					 else
						 fullComponents


	mainMenuFunc: function (m) {
		m.addAction(tmXtest)
		m.addAction(actionAbout)
		m.addAction(actionExit)
	}



	QLabel {
		id: loadLabel
		visible: !servers.readyResources
		anchors.centerIn: parent

		function setText(m) {
			var k = Object.keys(m)
			text = ""
			for (var i=0; i<k.length; i++) {
				if (text.length)
					text += "\n"
				text += k[i]+" : "+m[k[i]]
			}
		}
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
		enabled: control.isCurrentItem
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
				cosClient.socketSend(CosMessage.ClassUserInfo, "getServerInfo")
				cosClient.socketSend(CosMessage.ClassUserInfo, "getResources")
			} else if (connectionState === Client.Standby) {
				mainStack.pop(control)
			}
		}

	}

	onPageActivated: if (_firstRun) {
						 _firstRun = false
						 servers.doAutoConnect()
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
