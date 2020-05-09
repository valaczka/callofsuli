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


Page {
	id: page

	property bool _isFirst: true

	Servers {
		id: servers
		client: cosClient

		onServerListLoaded: listMenu.model = serverList
		onServerInfoUpdated: serverListReload()
	}

	property bool isFirstRun: true

	background: Rectangle {
		color: "black"
		Video {
			id: bgVideo
			anchors.fill: parent
			source: "qrc:/vid/bg.mov"
			autoPlay: true
			loops: MediaPlayer.Infinite
		}

		Label {
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.bottomMargin: 5
			anchors.rightMargin: 10
			text: "Call of Suli v"+cosClient.clientVersionMajor+"."+cosClient.clientVersionMinor

			color: CosStyle.colorPrimaryLighter
			font.pixelSize: CosStyle.pixelSize*0.8
		}
	}

	header: QToolBar {
		id: toolbar

		title: qsTr("Call of Suli szerver")

		backButton.visible: false


		QMenuButton {
			MenuItem {
				text: qsTr("Új szerver")
				onClicked:  editServer(-1)
			}

			MenuItem {
				text: qsTr("Offline mód")
				onClicked: JS.createPage("Offline", {}, page)
			}

			MenuSeparator {}

			MenuItem {
				text: qsTr("Névjegy")
				onClicked: {
					JS.dialogMessageInfo("Call of Suli",
										 qsTr("Verzió: ")+Qt.application.version+
										 "\n© 2012-2020 Valaczka János Pál"
										 )
				}
			}
			MenuItem {
				text: qsTr("Kilépés")
				onClicked: mainWindow.close()
			}
		}

	}



	QPagePanel {
		id: p

		title: qsTr("Szerverek")

		anchors.fill: parent
		maximumWidth: 600

		rightLoader.sourceComponent: QMenuButton {
			MenuItem {
				text: qsTr("Új szerver")
			}
		}

		QListItemDelegate {
			id: listMenu
			anchors.fill: parent

			onClicked: {
				var id = listMenu.model[listMenu.currentIndex].id
				if (id === -1)
					editServer(-1)
				else
					servers.serverConnect(id)
			}

			onLongPressed: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			onRightClicked: {
				listRigthMenu.modelIndex = index
				listRigthMenu.popup()
			}

			Keys.onPressed: {
				if (event.key === Qt.Key_Insert) {
					editServer(-1)
				} else if (event.key === Qt.Key_F4 && listMenu.currentIndex !== -1) {
					editServer(listMenu.model[listMenu.currentIndex].id)
				} else if (event.key === Qt.Key_Delete && listMenu.currentIndex !== -1) {
					deleteServer(listMenu.currentIndex)
				} else if (event.key === Qt.Key_F1) {
					var o = JS.createPage("MapEditor", {}, page)
					o.pagePopulated.connect(function() {
						o.map.loadFromFile("AAA.cosm")
						o.map.mapOriginalFile = "AAA.cosm"
						o.mapName = "AAA.cosm"
					})
				}
			}


		}

		QMenu {
			id: listRigthMenu

			property int modelIndex: -1


			MenuItem {
				text: qsTr("Csatlakozás")
				onClicked: if (listRigthMenu.modelIndex !== -1) {
							   servers.serverConnect(listMenu.model[listRigthMenu.modelIndex].id)
						   }

			}

			MenuItem {
				text: qsTr("Szerkesztés")
				onClicked: editServer(listMenu.model[listRigthMenu.modelIndex].id)
			}

			MenuItem {
				text: qsTr("Törlés")
				onClicked: deleteServer(listRigthMenu.modelIndex)

			}

			MenuItem {
				text: qsTr("Automata csatlakozás")
				onClicked: if (listRigthMenu.modelIndex !== -1) {
							   servers.serverSetAutoConnect(listMenu.model[listRigthMenu.modelIndex].id)
						   }
			}

			MenuSeparator {}

			MenuItem {
				text: qsTr("Új szerver")
				onClicked: editServer(-1)
			}
		}


	}


	Connections {
		target: cosClient

		onConnectionStateChanged: {
			if (connectionState === Client.Connected) {
				JS.createPage("MainMenu", {}, page)
			} else if (connectionState === Client.Standby) {
				mainStack.pop(page)
			}
		}
	}

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()

		if (_isFirst) {
			var autoConnectId = servers.serverListReload()
			_isFirst = false

			if (autoConnectId !== -1)
				servers.serverConnect(autoConnectId)
		}

		forceActiveFocus()
	}


	function deleteServer(idx) {
		var d = JS.dialogCreateQml("YesNo", {
									   title: qsTr("Biztosan törlöd a szervert?"),
									   text: idx === -1 ? "" : listMenu.model[idx].labelTitle
								   })
		d.accepted.connect(function () {
			servers.serverInfoDelete(listMenu.model[idx].id)
		})
		d.open()
	}

	function editServer(idx) {
		JS.createPage("ServerEdit",
					  {
						  servers: servers,
						  serverId: idx
					  },
					  page)

	}

	function windowClose() {
		return true
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
