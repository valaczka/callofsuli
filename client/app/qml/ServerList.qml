import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QTabContainer {
	id: panel

	title: qsTr("Szerverek")
	icon: "qrc:/internal/icon/desktop-tower.svg"
	menu: QMenu {
		MenuItem { action: actionServerNew }
		MenuItem { action: actionServerSearch }
		MenuItem { action: actionServerQR }
		MenuSeparator {}
		MenuItem { action: actionDemo }
		MenuItem { action: actionAbout }
		MenuItem { action: actionExit }
	}


	readonly property bool isDisconnected: cosClient.connectionState === Client.Standby || cosClient.connectionState === Client.Disconnected




	SortFilterProxyModel {
		id: userProxyModel
		sourceModel: servers.serversModel

		proxyRoles: [
			ExpressionRole {
				name: "displayName"
				expression: model.host+":"+model.port+(model.ssl ? " (SSL)" : "")+(model.broadcast ? qsTr(" [auto]") : "")
			}
		]
	}



	QObjectListView {
		id: serverList
		anchors.fill: parent

		visible: isDisconnected

		model: userProxyModel
		modelTitleRole: "displayName"
		modelSubtitleRole: "username"

		header: QTabHeader {
			tabContainer: panel
			isPlaceholder: true
		}

		autoSelectorChange: true

		delegateHeight: CosStyle.twoLineHeight

		leftComponent: QFlipable {
			id: flipable
			width: serverList.delegateHeight
			height: width

			frontIcon: model && model.broadcast ? "qrc:/internal/icon/access-point-network.svg" : "qrc:/internal/icon/access-point.svg"
			backIcon: "qrc:/internal/icon/access-point-check.svg"
			color: flipped ? CosStyle.colorAccent : CosStyle.colorPrimaryDark
			flipped: model && model.autoconnect

			mouseArea.onClicked: servers.serverSetAutoConnect(serverList.modelObject(modelIndex))
		}

		onClicked: servers.serverConnect(serverList.modelObject(index))

		onRightClicked: contextMenu.popup()
		onLongPressed: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionConnect }
			MenuItem { action: actionEdit}
			MenuItem { action: actionRemove }
			MenuSeparator {}
			MenuItem { action: actionAutoConnect }
		}


		onKeyInsertPressed: actionServerNew.trigger()
		onKeyF4Pressed: actionEdit.trigger()
		onKeyDeletePressed: actionRemove.trigger()
		onKeyF2Pressed: actionAutoConnect.trigger()
	}



	QToolButtonBig {
		anchors.centerIn: parent
		visible: !servers.serversModel.count
		action: actionServerNew
		color: CosStyle.colorOK
	}



	Column {
		anchors.centerIn: parent
		visible: !isDisconnected

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
			themeColors: CosStyle.buttonThemeRed
			text: qsTr("Mégsem")
			icon.source: "qrc:/internal/icon/close-circle.svg"
			onClicked: cosClient.closeConnection()
		}
	}



	Action {
		id: actionServerNew
		text: qsTr("Hozzáadás")
		icon.source: "qrc:/internal/icon/access-point-plus.svg"
		onTriggered: {
			servers.uiAdd()
		}
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		icon.source: "qrc:/internal/icon/connection.svg"
		enabled: serverList.currentIndex !== -1
		onTriggered: servers.serverConnect(serverList.modelObject(serverList.currentIndex))

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		icon.source: "qrc:/internal/icon/pencil.svg"
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			servers.uiEdit(serverList.modelObject(serverList.currentIndex))
		}
	}

	Action {
		id: actionRemove
		icon.source: "qrc:/internal/icon/access-point-remove.svg"
		text: qsTr("Törlés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			var more = servers.serversModel.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Szerverek törlése"),
												text: qsTr("Biztosan törlöd a kijelölt %1 szervert?").arg(more),
												image: "qrc:/internal/icon/access-point-remove.svg"
											})
				dd.accepted.connect(function () {
					servers.serverDeleteList(servers.serversModel.getSelected())
					servers.serversModel.unselectAll()
				})
				dd.open()
			} else {
				var o = serverList.modelObject(serverList.currentIndex)

				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Szerver törlése"),
											   text: qsTr("Biztosan törlöd a szervert?\n%1").arg(o.host+":"+o.port+(o.ssl ? " (SSL)" : "")),
											   image: "qrc:/internal/icon/access-point-remove.svg"
										   })
				d.accepted.connect(function () {
					servers.serverDelete(serverList.modelObject(serverList.currentIndex))
					servers.serversModel.unselectAll()
				})
				d.open()
			}
		}
	}


	Action {
		id: actionAutoConnect
		text: qsTr("Automata csatlakozás")
		icon.source: "qrc:/internal/icon/access-point-check.svg"
		enabled: serverList.currentIndex !== -1
		onTriggered:  {
			servers.serverSetAutoConnect(serverList.modelObject(serverList.currentIndex))
		}
	}


	Action {
		id: actionServerSearch
		text: qsTr("Keresés")
		icon.source: CosStyle.iconSearch
		onTriggered: {
			servers.sendBroadcast()
		}
	}


	Action {
		id: actionServerQR
		text: qsTr("Beolvasás")
		icon.source: "qrc:/internal/icon/qrcode-scan.svg"
		onTriggered: {
			cosClient.checkMediaPermissions()
		}
	}




	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		icon.source: "qrc:/internal/img/callofsuli_square.svg"
		onTriggered: {
			var dd = JS.dialogCreateQml("About", {})
			dd.open()
		}
	}

	Action {
		id: actionExit
		icon.source: "qrc:/internal/icon/application-export.svg"
		text: qsTr("Kilépés")
		onTriggered: mainWindow.close()
	}


	Action {
		id: actionDemo
		text: qsTr("Demo")
		icon.source: "qrc:/internal/icon/presentation-play.svg"
		onTriggered: {
			JS.createPage("Map", {
							  demoMode: true,
							  title: qsTr("Demo mód"),
							  readOnly: false
						  })
		}
	}


	Component {
		id: cmpQR
		ReadQR {
			onTagFound: {
				if (servers.isValidUrl(tag)) {
					captureEnabled = false
					panel.tabPage.stack.pop(panel)
					servers.parseUrl(tag)
				}
			}
		}
	}


	Connections {
		target: cosClient

		function onMediaPermissionsDenied() {

		}

		function onMediaPermissionsGranted() {
			pushContent(cmpQR, {})
		}
	}


	onPopulated: {
		var s = servers.takeUrlString()

		if (s !== "")
			servers.parseUrl(s)
		else
			serverList.forceActiveFocus()
	}


	backCallbackFunction: function () {
		if (servers.serversModel.selectedCount) {
			servers.serversModel.unselectAll()
			return true
		}

		return false
	}
}



