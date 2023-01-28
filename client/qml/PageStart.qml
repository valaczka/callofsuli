import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
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
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionAdd }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionDemo }
			Qaterial.MenuSeparator {}
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
			iconSource: Qaterial.Icons.desktopClassic
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
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionDelete }
		}

		onRightClickOrPressAndHold: {
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
					visible: Client.webSocket.state != WebSocket.Connected
				}

				Qaterial.LabelWithCaption {
					id: txt
					anchors.verticalCenter: parent.verticalCenter
					text: Client.webSocket.state == WebSocket.Connected ? qsTr("Csatlakozva") : qsTr("Csatlakozás...")
					caption: Client.server ? Client.server.url : ""
				}
			}

			Qaterial.RaisedButton {
				anchors.horizontalCenter: parent.horizontalCenter
				backgroundColor: Qaterial.Colors.red600
				foregroundColor: Qaterial.Colors.white
				text: qsTr("Megszakítás")
				icon.source: Qaterial.Icons.close
				onClicked: Client.webSocket.abort()
			}
		}

	}


	QFabButton {
		visible: view.visible
		action: actionAdd
	}


	Action {
		id: actionAdd
		text: qsTr("Hozzáadás")
		icon.source: Qaterial.Icons.plus
		onTriggered: Client.stackPushPage("ServerEdit.qml")
	}

	Action {
		id: actionEdit
		text: qsTr("Szerkesztés")
		icon.source: Qaterial.Icons.pencil
		enabled: view.currentIndex != -1
		onTriggered: Client.stackPushPage("ServerEdit.qml", {server: view.model.get(view.currentIndex)})
	}

	Action {
		id: actionDelete
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.trashCan
		enabled: view.currentIndex != 1 || view.selectEnabled
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
								iconSource: Qaterial.Icons.closeCircle
							})

			} else {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									Client.serverDelete(view.model.get(view.currentIndex))
								},
								text: qsTr("Biztosan törlöd a szervert?"),
								title: view.model.get(view.currentIndex).serverName,
								iconSource: Qaterial.Icons.closeCircle
							})
			}

		}
	}


	states: [
		State {
			name: "disconnected"
			when: Client.webSocket.state == WebSocket.Disconnected
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
			when: Client.webSocket.state != WebSocket.Disconnected
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

	StackView.onActivated: view.forceActiveFocus()
}
