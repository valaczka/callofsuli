import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (view.selectEnabled) {
			view.unselectAll()
			return false
		}
		return true
	}

	title: view.selectEnabled ? Client.serverListSelectedCount : "Call of Suli"

	appBar.backButtonVisible: false
	appBar.rightComponent: Qaterial.AppBarButton
	{
		visible: view.visible
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: (Qt.platform.os === "windows" || Qt.platform.os === "linux") ? menuDesktop.open() : menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionAdd }
			//QMenuItem { action: actionQR }
			Qaterial.MenuSeparator {}
			//			QMenuItem { action: actionPageDev }
			QMenuItem { action: actionDemo }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionSettings }
			QMenuItem { action: actionAbout }
			QMenuItem { action: actionExit }
		}

		QMenu {
			id: menuDesktop

			QMenuItem { action: actionAdd }
			//QMenuItem { action: actionQR }
			Qaterial.MenuSeparator {}
			//QMenuItem { action: actionPageDev }
			QMenuItem { action: actionDemo }
			QMenuItem { action: actionEditor }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionSettings }
			QMenuItem { action: actionAbout }
			QMenuItem { action: actionExit }
		}
	}


	Action {
		id: actionAbout
		text: qsTr("Névjegy")
		icon.source: "qrc:/internal/img/callofsuli_square.svg"
		onTriggered: {
			onClicked: Client.stackPushPage("PageAbout.qml", {})
		}
	}

	Action {
		id: actionExit
		icon.source: Qaterial.Icons.applicationExport
		text: qsTr("Kilépés")
		onTriggered: Client.closeWindow()
	}


	Action {
		id: actionDemo
		text: qsTr("Demo")
		icon.source: Qaterial.Icons.presentationPlay
		onTriggered: {
			onClicked: Client.loadDemoMap()
		}
	}

	Action {
		id: actionSettings
		text: qsTr("Beállítások")
		icon.source: Qaterial.Icons.cog
		onTriggered: {
			onClicked: Client.stackPushPage("PageStudentSettings.qml", {})
		}
	}

	Action {
		id: actionEditor
		text: qsTr("Pályaszerkesztő")
		icon.source: Qaterial.Icons.briefcaseEdit
		onTriggered: {
			onClicked: Client.stackPushPage("PageMapEditor.qml", {})
		}
	}



	QListView {
		id: view

		currentIndex: -1
		anchors.fill: parent
		visible: false
		autoSelectChange: true

		model: Client.serverList

		delegate: QItemDelegate {
			property Server server: model.qtObject
			selectableObject: server

			highlighted: ListView.isCurrentItem
			highlightedIcon: server ? server.autoConnect : false
			iconSource: server.offlineEngine && server.offlineEngine.engineState == OfflineClientEngine.EngineActive ?
							Qaterial.Icons.clockCheck :
							Qaterial.Icons.desktopClassic
			text: server ? server.serverName : ""
			secondaryText: server ? (server.user.username.length ? server.user.username + " @ " : "") + server.url
								  : ""

			onClicked: if (!view.selectEnabled)
						   Client.connectToServer(server)
		}

		Qaterial.Menu {
			id: contextMenu
			QMenuItem { action: view.actionSelectAll }
			QMenuItem { action: view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionAdd }
			QMenuItem { action: actionEdit }
			QMenuItem { action: actionAutoConnect }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionDelete }
		}

		onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
										if (index != -1)
										currentIndex = index
										contextMenu.popup(mouseX, mouseY)
									}
	}


	Item {
		id: connectingItem
		anchors.fill: parent
		visible: false

		Column {

			anchors.centerIn: parent
			spacing: 20

			Row {
				spacing: 20

				Qaterial.BusyIndicator {
					anchors.verticalCenter: parent.verticalCenter
					height: txt.height
					width: txt.height
					visible: Client.httpConnection.state != HttpConnection.Connected
				}

				Qaterial.LabelWithCaption {
					id: txt
					anchors.verticalCenter: parent.verticalCenter
					text: Client.httpConnection.state == HttpConnection.Connected ? qsTr("Csatlakozva") : qsTr("Csatlakozás...")
					caption: Client.server ? Client.server.url : ""
				}
			}

			Qaterial.RaisedButton {
				anchors.horizontalCenter: parent.horizontalCenter
				backgroundColor: Qaterial.Colors.red600
				foregroundColor: Qaterial.Colors.white
				text: qsTr("Megszakítás")
				icon.source: Qaterial.Icons.close
				onClicked: Client.httpConnection.abort()
			}
		}

	}


	Qaterial.Banner
	{
		anchors.top: parent.top
		width: parent.width
		drawSeparator: true
		text: qsTr("Még egyetlen szerver sincsen felvéve, adj hozzá egyet")
		iconSource: Qaterial.Icons.desktopClassic
		fillIcon: false
		outlinedIcon: true
		highlightedIcon: true

		//action1: qsTr("QR-kód")
		action1: qsTr("Hozzáadás")

		//onAction1Clicked: actionQR.trigger()
		onAction1Clicked: actionAdd.trigger()

		enabled: !Client.serverList.length
		visible: !Client.serverList.length
	}

	/*Column {
		visible: !Client.serverList.length
		anchors.centerIn: parent

		spacing: 5 * Qaterial.Style.pixelSizeRatio

		Qaterial.LabelCaption {
			text: qsTr("Betűméret:")
			anchors.horizontalCenter: parent.horizontalCenter
		}

		Row {
			anchors.horizontalCenter: parent.horizontalCenter
			Qaterial.ToolButton {
				action: Client.mainWindow.fontMinus
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontNormal
				display: AbstractButton.IconOnly
			}

			Qaterial.ToolButton {
				action: Client.mainWindow.fontPlus
				display: AbstractButton.IconOnly
			}
		}
	}*/


	QDashboardButton {
		visible: !Client.serverList.length
		anchors.centerIn: parent

		action: actionAdd
		text: qsTr("Új szerver")
		icon.source: action.icon.source
		/*highlighted: false
		outlined: true
		flat: true

		textColor: Qaterial.Colors.yellow400

		onClicked: Client.stackPushPage("PageMarket.qml")*/
	}

	QFabButton {
		visible: Client.serverList.length
		action: actionAdd
	}


	Action {
		id: actionAdd
		text: qsTr("Hozzáadás")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			if (Client.authorizedServers.length === 0) {
				Client.stackPushPage("ServerEdit.qml")
				return
			}

			_authServersModel.clear()

			for (let i=0; i<Client.authorizedServers.length; ++i) {
				let l = Client.authorizedServers[i]
				_authServersModel.append({
							  text: l.name,
							  secondaryText: l.url,
							  icon: Qaterial.Icons.desktopClassic,
							  url: l.url
						  })
			}

			_authServersModel.append({
						  text: qsTr("-- saját szerver --"),
						  secondaryText: "",
						  icon: Qaterial.Icons.serverPlusOutline,
						  url: ""
					  })


			Qaterial.DialogManager.openListView(
						{
							onAccepted: function(index)
							{
								if (index < 0)
									return

								let server = _authServersModel.get(index)

								if (server.url === "") {
									Client.stackPushPage("ServerEdit.qml")
									return
								} else {
									let s = Client.serverAdd()
									s.serverName = server.text
									s.url = server.url
									Client.serverSetAutoConnect(s)
									Client.connectToServer(s)
								}
							},
							title: qsTr("Új szerver hozzáadása"),
							standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
							model: _authServersModel
						})

		}
	}

	ListModel {
		id: _authServersModel
	}

	/*Action {
		id: actionQR
		text: qsTr("QR-kód beolvasás")
		icon.source: Qaterial.Icons.qrcodeScan
		onTriggered: Client.Utils.checkMediaPermissions()
	}*/

	Action {
		id: actionPageDev
		text: qsTr("_PageDev")
		icon.source: Qaterial.Icons.developerBoard
		onTriggered: {
			onClicked: Client.stackPushPage("_PageDev.qml", {})
		}
	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		icon.source: Qaterial.Icons.pencil
		enabled: view.currentIndex != -1
		onTriggered: Client.stackPushPage("ServerEdit.qml", {server: view.model.get(view.currentIndex)})
	}

	Action {
		id: actionAutoConnect
		text: qsTr("Automatikus csatlakozás")
		icon.source: Qaterial.Icons.connection
		enabled: view.currentIndex != -1
		onTriggered: {
			Client.serverSetAutoConnect(view.model.get(view.currentIndex))
			view.unselectAll()
		}
	}

	Action {
		id: actionDelete
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.delete_
		enabled: view.currentIndex != -1 || view.selectEnabled
		onTriggered: {
			if (view.selectEnabled) {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									Client.serverDeleteSelected()
									view.checkSelected()
								},
								text: qsTr("Biztosan törlöd a kijelölt %1 szervert?").arg(Client.serverListSelectedCount),
								title: qsTr("Szerverek törlése"),
								iconSource: Qaterial.Icons.delete_
							})

			} else {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									Client.serverDelete(view.model.get(view.currentIndex))
									view.checkSelected()
								},
								text: qsTr("Biztosan törlöd a szervert?"),
								title: view.model.get(view.currentIndex).serverName,
								iconSource: Qaterial.Icons.delete_
							})
			}

		}
	}


	states: [
		State {
			name: "disconnected"
			when: Client.httpConnection.state == HttpConnection.Disconnected
			PropertyChanges {
				target: view
				visible: true
			}
			PropertyChanges {
				target: connectingItem
				visible: false
			}
		},
		State {
			name: "connecting"
			when: Client.httpConnection.state != HttpConnection.Disconnected
			PropertyChanges {
				target: view
				visible: false
			}
			PropertyChanges {
				target: connectingItem
				visible: true
			}
		}
	]

	StackView.onActivated: {
		view.forceActiveFocus()
		Client.contextHelper.setCurrentContext(ContextHelperData.ContextStart)
	}

	StackView.onActivating: {
		Client.safeMarginsGet()
	}


	/*Connections {
		target: Client.Utils

		function onMediaPermissionsGranted() {
			var page = Client.stackPushPage("PageReadQR.qml")
			page.tagFound.connect(function(tag) {
				var server = Client.serverAddWithUrl(tag)

				if (server) {
					page.captureEnabled = false
					Client.setParseUrl(tag)
					Client.stackPop(page)
					Client.connectToServer(server)
				} else {
					Client.snack(qsTr("Érvénytelen URL"))
				}
			})
		}
	}*/
}
