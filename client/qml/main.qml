import QtQuick 2.12
import QtQuick.Controls 2.12
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Qaterial.ApplicationWindow
{
	id: mainWindow
	width: 640
	height: 480

	visible: true

	//title: (cosClient.connectionState !== Client.Standby && cosClient.serverName.length ? cosClient.serverName+" - " : "") + "Call of Suli"

	minimumWidth: 320
	minimumHeight: 240

	MainStackView {
		id: mainStackView
		anchors.fill: parent
	}


	Component.onCompleted:
	{
		JS.intializeStyle()
		Client.mainStack = mainStackView
		Client.mainWindow = mainWindow

		/// Ez kéne máshova
		Client.stackPushPage("PageStart.qml", {})
	}


	Shortcut
	{
		sequences: ["Esc", "Back"]
		onActivated:
		{
			Client.stackPop()
		}
	}


	MouseArea {
		anchors.fill: parent
		acceptedButtons: Qt.NoButton
		onWheel: {
			if (wheel.modifiers & Qt.ControlModifier) {
				var i = wheel.angleDelta.y/120
				if (i>0)
					fontPlus.trigger()
				else if (i<0)
					fontMinus.trigger()

				wheel.accepted = true
			} else {
				wheel.accepted = false
			}
		}
	}


	Action {
		id: fontPlus
		shortcut: "Ctrl++"
		text: qsTr("Növelés")
		icon.source: Qaterial.Icons.magnifyPlus
		onTriggered: Client.pixelSize++
	}

	Action {
		id: fontMinus
		shortcut: "Ctrl+-"
		text: qsTr("Csökkentés")
		icon.source: Qaterial.Icons.magnifyMinus
		onTriggered: Client.pixelSize--
	}

	Action {
		id: fontNormal
		shortcut: "Ctrl+0"
		text: qsTr("Visszaállítás")
		icon.source: Qaterial.Icons.magnifyRemoveOutline
		onTriggered: Client.resetPixelSize()
	}


	Action {
		id: actionFullscreen
		shortcut: "Ctrl+F11"

		onTriggered: {
			if (mainWindow.visibility === Window.FullScreen)
				mainWindow.showMaximized()
			else
				mainWindow.showFullScreen()
		}
	}


	onClosing: {
		if (Client.closeWindow()) {
			close.accepted = true
			Qt.quit()
		} else {
			close.accepted = false
		}
	}


	function closeQuestion(_text : string) {
		Qaterial.DialogManager.showDialog(
					{
						onAccepted: function()
						{
							Client.closeWindow(true)
						},
						text: _text,
						title: qsTr("Kilépés"),
						iconSource: Qaterial.Icons.account,
						standardButtons: Dialog.No | Dialog.Yes
					})
	}
}

