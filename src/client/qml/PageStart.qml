import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: control

	requiredPanelWidth: 900

	defaultTitle: qsTr("Call of Suli")
	mainToolBar.backButton.visible: false

	property bool _firstRun: true
	readonly property bool isDisconnected: cosClient.connectionState == Client.Standby || cosClient.connectionState == Client.Disconnected

	mainToolBar.visible: isDisconnected

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



		onResourceReady: {
			JS.createPage("MainMenu", {})
			servers.serverTryLogin(servers.connectedServerKey)
		}

		Component.onCompleted: serverListReload()
	}

	property list<Component> fullComponents: [
		Component { ServerList {
				panelVisible: true
				layoutFillWidth: !servers.editing
			} },
		Component { ServerEdit {
				panelVisible: servers.editing
			} }
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
		m.addAction(test)
		m.addAction(tmXtest)
		m.addAction(actionAbout)
		m.addAction(actionExit)
	}


	Column {
		anchors.centerIn: parent
		visible: !control.isDisconnected

		spacing: 10

		Row {
			spacing: 10
			anchors.horizontalCenter: parent.horizontalCenter


			BusyIndicator {
				anchors.verticalCenter: parent.verticalCenter
				height: CosStyle.pixelSize*3
				width: CosStyle.pixelSize*3
				running: true
				Material.accent: CosStyle.colorPrimaryLighter
			}

			QLabel {
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("Kapcsolódás...")
				font.pixelSize: CosStyle.pixelSize*1.2
				color: CosStyle.colorPrimary
			}

		}

		QButton {
			anchors.horizontalCenter: parent.horizontalCenter
			themeColors: CosStyle.buttonThemeDelete
			text: qsTr("Mégsem")
			icon.source: CosStyle.iconCancel
			onClicked: cosClient.closeConnection()
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
		text: "Map Editor"
		enabled: isCurrentItem
		onTriggered: {
			var o = JS.createPage("MapEditor", {}) //loadFileName: "/tmp/ttt.dat" })
		}
	}


	Action {
		id: tmXtest
		shortcut: "F2"
		text: "TMX"
		enabled: isCurrentItem
		onTriggered: {
			var o = JS.createPage("TMXtest", {})
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
