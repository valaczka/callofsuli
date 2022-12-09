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
	//title: "Qaterial Gallery"

	menuBar: Qaterial.MenuBar
	{
		visible: true

		onPrimary: true

		Qaterial.Menu
		{
			title: qsTr("File")
			width: 300

			backgroundColor: Qt.darker(Qaterial.Colors.cyan900)

			Qaterial.MenuItem
			{
				text: qsTr("New...");onTriggered: Client.addPage();
				action: Action { shortcut: "Ctrl+N" }
			} // MenuItem
			Qaterial.MenuItem { text: qsTr("Open...");onTriggered: console.log("Open") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Save");onTriggered: console.log("Save") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Save As...");onTriggered: console.log("Save As") } // MenuItem
			Qaterial.MenuSeparator { width: parent.width } // MenuItem
			Qaterial.MenuItem { text: qsTr("Quit");onTriggered: console.log("Quit") } // MenuItem
		} // Menu

		Qaterial.Menu
		{
			width: 300
			title: qsTr("Edit")


			Qaterial.MenuItem
			{
				text: qsTr("Copy");
				//icon.source: "qrc:/QaterialGallery/images/icons/content-copy.svg";
				action: Action
				{
					shortcut: "Ctrl+C"
					onTriggered: console.log("Copy")
				}

			} // MenuItem
			Qaterial.MenuItem { text: qsTr("Cut");onTriggered: console.log("Cut") } // MenuItem
			Qaterial.MenuItem { text: qsTr("Paste");onTriggered: console.log("Paster") } // MenuItem

			Qaterial.Menu
			{
				title: "Find/Replace"
				Qaterial.MenuItem { text: "Find Next" } // MenuItem
				Qaterial.MenuItem { text: "Find Previous" } // MenuItem
				Qaterial.MenuItem { text: "Replace" } // MenuItem
			} // Menu

			Qaterial.MenuSeparator { width: parent.width } // MenuSeperator

			Qaterial.MenuItem
			{
				text: qsTr("Dummy");
				icon.source: Qaterial.Icons.airplane
				//icon.source: "qrc:/QaterialGallery/images/icons/airplane.svg";
				action: Action
				{
					shortcut: "Ctrl+Shift+F5"
					onTriggered: Client.pixelSize--
				}
			} // MenuItem
			Qaterial.MenuItem
			{
				text: qsTr("Colored Icon");
				icon.source: Qaterial.Icons.album
				icon.color: Qaterial.Style.accentColor
				action: Action
				{
					shortcut: "Ctrl+K,Ctrl+L"
					onTriggered: console.log("Colored")
				}
			} // MenuItem
		} // Menu

		Qaterial.Menu
		{

			title: qsTr("Test")
			Qaterial.MenuItem { text: qsTr("Checked 1 very loing afznuaefb");checked: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 2");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 3");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 4");checkable: true } // MenuItem
			Qaterial.MenuItem { text: qsTr("Checked 5");checkable: true } // MenuItem
		} // Menu

		Qaterial.Menu
		{
			title: qsTr("Nézet")
			width: 400
			Qaterial.MenuItem { icon.source: action.icon.source; action: fontPlus }
			Qaterial.MenuItem { icon.source: action.icon.source; action: fontMinus }
			Qaterial.MenuItem { icon.source: action.icon.source; action: fontNormal }
		} // Menu
	} // MenuBar



	MainStackView {
		id: mainStackView
		anchors.fill: parent
	}


	Component.onCompleted:
	{
		JS.intializeStyle()
		Client.mainStack = mainStackView

		Client.mainStack.createPage("PageStart.qml", {})
	}


	Shortcut
	{
		sequences: ["Esc", "Back"]
		enabled: Client.mainStack.depth > 1
		onActivated:
		{
			Client.mainStack.pop()
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

			Qaterial.Style.menuItem.iconWidth
		}
	}


}

