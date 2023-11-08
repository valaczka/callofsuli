import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "../QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "../JScript.js" as JS
import ".."

QPage {
	id: control

	title: "Call of Suli"

	appBar.backButtonVisible: false
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionDemo }
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

	Item {
		id: connectingItem
		anchors.fill: parent

		Column {
			anchors.centerIn: parent
			spacing: 20

			Row {
				spacing: 20

				visible: Client.httpConnection.state != HttpConnection.Disconnected

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
				visible: Client.httpConnection.state != HttpConnection.Disconnected

				anchors.horizontalCenter: parent.horizontalCenter
				backgroundColor: Qaterial.Colors.red600
				foregroundColor: Qaterial.Colors.white
				text: qsTr("Megszakítás")
				icon.source: Qaterial.Icons.close
				onClicked: Client.httpConnection.close()
			}

			Qaterial.IconLabelWithCaption {
				visible: Client.httpConnection.state == HttpConnection.Disconnected
				icon.source: Qaterial.Icons.connection
				text: qsTr("Nincs kapcsolat")
				caption: qsTr("Nem lehet csatlakozni a szerverhez")
			}

			Qaterial.RaisedButton {
				visible: Client.httpConnection.state == HttpConnection.Disconnected
				anchors.horizontalCenter: parent.horizontalCenter
				backgroundColor: Qaterial.Colors.red600
				foregroundColor: Qaterial.Colors.white
				text: qsTr("Kilépés")
				icon.source: Qaterial.Icons.applicationExport
				onClicked: Client.mainWindow.close()
			}
		}

	}


	StackView.onActivated: if (Client.server)
							   Client.httpConnection.connectToServer()
}
