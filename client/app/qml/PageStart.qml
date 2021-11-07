import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: control

	defaultTitle: qsTr("Call of Suli")
	mainToolBar.backButton.visible: stackComponent.stackView.depth > 1

	activity: Servers {
		id: servers

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
			cosClient.reloadGameResources()
			JS.createPage("MainMenu", {
							  servers: servers
						  })
		}


		onCertificateError: {
			var d = JS.dialogCreateQml("YesNoFlickable", {
										   title: qsTr("Hibás tanúsítvány"),
										   text: data.errorStrings.join("\n")
												 +qsTr("\n\nEnnek ellenére megbízol a tanúsítványban?"),
										   details: qsTr("Tanúsítvány részletei:\n")+data.info
									   })

			d.item.titleColor = CosStyle.colorWarningLighter
			d.item.textColor = CosStyle.colorWarningLight

			d.accepted.connect(function() {
				servers.acceptCertificate(data.serverKey, certificate, data.errorCodes)
			})

			d.open()
		}


		function uiEdit(key) {
			stackComponent.pushComponent(panelEdit, {serverKey: key})
		}

		function uiAdd() {
			stackComponent.pushComponent(panelEdit, {serverKey: -1})
		}

		function uiBack() {
			stackComponent.layoutBack()
		}

		Component.onCompleted: serverListReload()
	}


	Component {
		id: panelEdit
		ServerEdit {}
	}



	mainMenuFunc: function (m) {
		m.addAction(actionDemo)
		m.addAction(actionAbout)
		m.addAction(actionExit)
	}


	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		//requiredWidth: 500

		//headerContent: QLabel {	}

		initialItem: ServerList {  }
	}



	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		onTriggered: {
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

	onPageActivatedFirst:  servers.doAutoConnect(cosClient.takePositionalArgumentsToProcess())


	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}
}
