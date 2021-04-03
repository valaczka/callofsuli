import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: control

	defaultTitle: qsTr("Call of Suli")
	mainToolBar.backButton.visible: pageStack.depth > 1

	property bool _firstRun: true

	activity: Servers {
		id: servers

		property bool editing: false
		property int serverKey: -1

		onResourceDownloadRequest: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Letöltés"),
										   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize)
									   })
			d.accepted.connect(function() {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: servers.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			})

			d.rejected.connect(function() {
				cosClient.closeConnection()
			})

			d.open()
		}

		onEditingChanged: if (editing && stackMode) {
							  addStackPanel(panelEdit)
						  }


		onResourceReady: {
			cosClient.reloadGameResources()
			JS.createPage("MainMenu", {})
			servers.serverTryLogin(servers.connectedServerKey)
		}


		Component.onCompleted: serverListReload()
	}

	panelComponents: [
		Component { ServerList {
				layoutFillWidth: !servers.editing
			} },
		Component { ServerEdit {
				panelVisible: servers.editing
			} }
	]


	Component {
		id: panelEdit
		ServerEdit {}
	}


	onStackModeChanged: {
		servers.editing = false
		servers.serverKey = -1
	}



	mainMenuFunc: function (m) {
		m.addAction(actionDemo)
		m.addAction(actionAbout)
		m.addAction(actionExit)
	}



	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		onTriggered: {
			/*JS.dialogMessageInfo("Call of Suli",
								 qsTr("Verzió: ")+Qt.application.version+
								 "\n© 2012-2021 Valaczka János Pál"
								 )*/

			var dd = JS.dialogCreateQml("About", {})
			dd.open()
		}
	}

	Action {
		id: actionExit
		text: qsTr("Kilépés")
		onTriggered: mainWindow.close()
	}


	Action {
		id: actionDemo
		text: qsTr("Demo")
		onTriggered: {
			JS.createPage("MapMissions", {
							  demoMode: true,
							  defaultSubTitle: qsTr("Demo mód")
						  })
		}
	}



	Connections {
		target: cosClient

		function onConnectionStateChanged(connectionState) {
			if (connectionState === Client.Standby) {
				mainStack.pop(control)
			}
		}

	}

	onPageActivated: if (_firstRun) {
						 _firstRun = false
						 servers.doAutoConnect()
					 }


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (servers.editing) {
			servers.editing = false
			return true
		}

		return false
	}
}
