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


	property bool _completed: false
	readonly property bool _allLoaded: mainStackView._loaded && _completed

	MainStackView {
		id: mainStackView
		anchors.fill: parent

		property bool _loaded: false

		onStackViewLoaded: _loaded = true
	}


	Component.onCompleted:
	{
		JS.intializeStyle()

		if (Qt.platform.os != "wasm") {
			Qaterial.Style.dialog.implicitWidth = Qt.binding(function() {
				return Math.min(mainWindow.width*.9, 400 * Qaterial.Style.pixelSizeRatio)
			})
		}


		Client.mainStack = mainStackView
		Client.mainWindow = mainWindow

		_completed = true
	}


	on_AllLoadedChanged: if (_allLoaded) Client.onApplicationStarted()


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
		onTriggered: setPixelSize(Qaterial.Style.pixelSize+1)
	}

	Action {
		id: fontMinus
		shortcut: "Ctrl+-"
		text: qsTr("Csökkentés")
		icon.source: Qaterial.Icons.magnifyMinus
		onTriggered: setPixelSize(Qaterial.Style.pixelSize-1)
	}

	Action {
		id: fontNormal
		shortcut: "Ctrl+0"
		text: qsTr("Visszaállítás")
		icon.source: Qaterial.Icons.magnifyRemoveOutline
		onTriggered: Qaterial.Style.pixelSize = Qaterial.Style.defaultPixelSize
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


	onClosing: { if (Client.closeWindow()) {
			if (mainStackView.currentItem.onPageClose) {
				console.info(qsTr("Lap bezárási funkció meghívása:"), mainStackView.currentItem)
				mainStackView.currentItem.onPageClose()
			}
			close.accepted = true
			Qt.quit()
		} else {
			close.accepted = false
		}
	}



	function messageDialog(_text : string, _title : string, _type : string) {
		var _icon = Qaterial.Icons.informationOutline
		var _color = Qaterial.Style.primaryTextColor()

		if (_type === "warning") {
			_icon = Qaterial.Icons.alert
			_color = Qaterial.Colors.orange500
		} else if (_type === "error") {
			_icon = Qaterial.Icons.alertOctagon
			_color = Qaterial.Colors.red600
		}

		Qaterial.DialogManager.showDialog(
					{
						text: _text,
						title: _title,
						iconSource: _icon,
						iconColor: _color,
						textColor: _color,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
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
						iconSource: Qaterial.Icons.closeCircle,
						iconColor: Qaterial.Colors.orange500,
						textColor: Qaterial.Colors.orange500,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: Dialog.No | Dialog.Yes
					})
	}


	function snack(_text : string) {
		Qaterial.SnackbarManager.show(
					{
						text: _text,
					}

		)
	}


	function setPixelSize(newSize) {
		if (newSize >= Qaterial.Style.defaultPixelSize/2.5 && newSize <= Qaterial.Style.defaultPixelSize * 3.0)
			Qaterial.Style.pixelSize = newSize
	}


}

