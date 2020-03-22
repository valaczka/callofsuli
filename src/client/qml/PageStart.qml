import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls.Material 2.3
import QtQuick.Layouts 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	Servers {
		id: servers
		client: cosClient

		onServerListLoaded: JS.setModel(listMenu.model, serverList)

	}

	property bool isFirstRun: true

	header: QToolBar {
		id: toolbar

		backButton.visible: false

		rightLoader.sourceComponent: Component {
			QMenuButton {
				MenuItem {
					text: qsTr("Új szerver")
					onClicked:  createServer()
				}

				MenuSeparator {}

				MenuItem {
					text: qsTr("Névjegy")
					onClicked: {
						JS.dialogMessageInfo("Call of Suli",
											 qsTr("Verzió: ")+Qt.application.version+
											 qsTr("\n© 2012-2019 Valaczka János Pál")
											 )
					}
				}
				MenuItem {
					text: qsTr("Kilépés")
					onClicked: mainWindow.close()
				}

				MenuItem {
					text: qsTr("QRTester")
					onClicked: JS.createPage("QRread", {})
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



	QPagePanel {
		id: p

		maximumWidth: 600

		blurSource: bgImage

		QListButtonDelegate {
			id: listMenu
			anchors.fill: parent

			onClicked: connectServer(listMenu.model.get(index).filePath)

			onLongPressed: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			onRightClicked: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}


		}

		QMenu {
			id: listRigthMenu

			property int modelIndex: -1


			MenuItem {
				text: qsTr("Kapcsolódás")
				onClicked: if (listRigthMenu.modelIndex !== -1) {
							   connectServer(listMenu.model.get(listRigthMenu.modelIndex).filePath)
						   }

			}

			MenuItem {
				text: qsTr("Szerkesztés")
				onClicked: editServer(listRigthMenu.modelIndex)
			}

			MenuItem {
				text: qsTr("Törlés")
				onClicked: deleteServer(listRigthMenu.modelIndex)
			}

			MenuItem {
				text: qsTr("Új szerver")
				onClicked: createServer()
			}
		}

		BusyIndicator {
			id: busy
			anchors.centerIn: parent
			running: false
		}
	}


	Component {
		id: dlgDelete

		QDialogYesNo {
			id: dlgYesNo

			property int idx: listMenu.currentIndex

			title: qsTr("Biztosan törlöd a szervert?")

			text: idx === -1 ? "" : listMenu.model.get(idx).label

			onDlgAccept: {
				page.deleteServerReal(idx)
			}

		}
	}



	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.title = qsTr("Call of Suli szerverek")

		servers.serverListReload()
	}


	function createServer() {
		var o = JS.createPageOnly("ConnectionModify",
								  {
									  connectionModel: listMenu.model,
									  connectionModelIndex: -1
								  })

		if (o) {
			o.saveData.connect(function(_data) {
				mainStack.pop(page.StackView.index)
				var r = Client.clientRunnable("connectionCreate", busy)

				r.parameters = {
					data: _data
				}

				r.success.connect(function() {
					var m = r.getResult()

				})

				r.finished.connect(function() { mainStack.pop(page) })

				r.start()
			})

			mainStack.push(o)
		}
	}




	function editServer(idx) {
		if (idx === -1) {
			console.error("currentIndex error")
			return
		}

		var o = JS.createPage("ConnectionModify",
								  {
									  servers: servers,
									  connectionModel: listMenu.model,
									  connectionModelIndex: idx
								  })
/*
		if (o) {
			o.saveData.connect(function(_data) {
				mainStack.pop(page.StackView.index)
				var r = Client.clientRunnable("connectionUpdate", busy)

				r.parameters = {
					filePath: listMenu.model.get(idx).filePath,
					data: _data
				}

				r.success.connect(function() {
					var m = r.getResult()

				})

				r.finished.connect(function() { mainStack.pop(page) })

				r.start()
			})

			mainStack.push(o)
		} */
	}


	function deleteServer(idx) {
		var d = JS.dialogCreate(dlgDelete)
		d.open()
	}


	function deleteServerReal(idx) {
		if (idx === -1) {
			console.error("currentIndex error")
			return
		}

		servers.deleteServer(listMenu.model.get(idx).filePath)
	}



	function connectServer(filepath) {
		var p = JS.createPage("Server", {
								  filePath: filepath,
							  })
		p.openFailed.connect(function() {
			if (Client.getSetting("autoConnect") === filepath)
				Client.setSetting("autoConnect", null)
		})

	}


	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		/* BACK */

		return false
	}
}
