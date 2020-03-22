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
	icon: CosStyle.iconComputer
	menu: QMenu {
		MenuItem { action: actionServerNew }
		MenuItem { action: actionServerSearch }
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

			frontIcon: CosStyle.iconComputer
			backIcon: CosStyle.iconComputer
			color: flipped ? CosStyle.colorAccent : CosStyle.colorPrimaryDark
			flipped: model.autoconnect

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
			icon.source: CosStyle.iconCancel
			onClicked: cosClient.closeConnection()
		}
	}



	Action {
		id: actionServerNew
		text: qsTr("Hozzáadás")
		icon.source: CosStyle.iconAdd
		onTriggered: {
			servers.uiAdd()
		}
	}

	Action {
		id: actionConnect
		text: qsTr("Csatlakozás")
		enabled: serverList.currentIndex !== -1
		onTriggered: servers.serverConnect(serverList.modelObject(serverList.currentIndex))

	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			servers.uiEdit(serverList.modelObject(serverList.currentIndex))
		}
	}

	Action {
		id: actionRemove
		icon.source: CosStyle.iconDelete
		text: qsTr("Törlés")
		enabled: serverList.currentIndex !== -1
		onTriggered: {
			var more = servers.serversModel.selectedCount

			if (more > 0) {
				var dd = JS.dialogCreateQml("YesNo", {
												title: qsTr("Szerverek törlése"),
												text: qsTr("Biztosan törlöd a kijelölt %1 szervert?").arg(more)
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
											   text: qsTr("Biztosan törlöd a szervert?\n%1").arg(o.name)
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
			JS.createPage("Map", {
							  demoMode: true,
							  title: qsTr("Demo mód"),
							  readOnly: false
						  })
		}
	}



	onPopulated: serverList.forceActiveFocus()
}



