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

		Qaterial.Style.primaryColorDark = Qaterial.Colors.cyan700
		Qaterial.Style.accentColorDark = Qaterial.Colors.amber500
		Qaterial.Style.backgroundColor = Qaterial.Colors.black
		Qaterial.Style.foregroundColorDark = Qaterial.Colors.cyan200
		Qaterial.Style.buttonTextColor = Qaterial.Colors.black

		Qaterial.Style.darkColorTheme.background = Qaterial.Style.backgroundColor
		Qaterial.Style.darkColorTheme.primary = Qaterial.Colors.cyan500
		Qaterial.Style.darkColorTheme.accent = Qaterial.Style.accentColorDark

		Client.Style.colorGlow = Qaterial.Colors.amber200
		Client.Style.colorEnemyGlow = Qaterial.Colors.deepOrangeA700

		Client.mainStack = mainStackView
		Client.mainWindow = mainWindow
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



	function messageDialog(_text : string, _title : string, _icon : string) {
		Qaterial.DialogManager.showDialog(
					{
						text: _text,
						title: _title,
						iconSource: _icon,
						iconColor: "red",
						standardButtons: Dialog.Ok
					})
	}


	function closeQuestion(_text : string, _pop : bool, _index: int) {
		Qaterial.DialogManager.showDialog(
					{
						onAccepted: function()
						{
							if (_pop)
								Client.stackPop(_index, true)
							else
								Client.closeWindow(true)
						},
						text: _text,
						title: _pop ? qsTr("Bezárás") : qsTr("Kilépés"),
						iconSource: Qaterial.Icons.account,
						standardButtons: Dialog.No | Dialog.Yes
					})
	}
}

