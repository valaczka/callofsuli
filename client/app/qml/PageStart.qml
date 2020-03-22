import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	title: qsTr("Call of Suli szerverek")
	backFunction: null

	activity: Servers {
		id: servers

		onResourceDownloadRequest: {
			if (servers.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
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
			} else {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: servers.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			}
		}


		onResourceReady: {
			cosClient.reloadGameResources()
			JS.createPage("Main", {
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
				servers.acceptCertificate(server, certificate, data.errorCodes)
			})

			d.open()
		}


		function uiEdit(key) {
			pushContent(panelEdit, {server: key})
		}

		function uiAdd() {
			pushContent(panelEdit, {server: null})
		}

		function uiBack() {
			stack.pop()
		}

		Component.onCompleted: serverListReload()
	}


	Component {
		id: panelEdit
		ServerEdit {  }
	}


	Component {
		id: panelList
		ServerList {  }
	}



	Connections {
		target: cosClient

		function onConnectionStateChanged(connectionState) {
			if (connectionState === Client.Standby) {
				mainStack.pop(control)
			}
		}

	}

	Component.onCompleted: {
		if (Qt.platform.os === "android" || Qt.platform.os === "ios") {
			mainWindow.visibility = "FullScreen"
		}
		pushContent(panelList)
	}

	onPageActivatedFirst:  servers.doAutoConnect(cosClient.takePositionalArgumentsToProcess())

}
