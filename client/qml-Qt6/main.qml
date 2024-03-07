import QtQuick
import QtQuick.Controls
import QtQuick.Window
import Box2D 2.0
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


// ----- For QtDeployer

import Qt.labs.folderlistmodel
import Qt.labs.platform
import Qt.labs.settings
import Qt.labs.qmlmodels
import Qt.labs.sharedimage

// -----

Qaterial.ApplicationWindow
{
	id: mainWindow
	width: 640
	height: 480

	visible: true

	minimumWidth: 320
	minimumHeight: 240

	color: "black"

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

		setPixelSize(Client.Utils.settingsGet("window/fontSize", Qaterial.Style.defaultPixelSize))

		Qaterial.Style.dialog.implicitWidth = Qt.binding(function() {
			return Math.min(mainWindow.width*.9, 400 * Qaterial.Style.pixelSizeRatio)
		})

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


	property Action fontPlus: Action {
		shortcut: "Ctrl++"
		text: qsTr("Növelés")
		icon.source: Qaterial.Icons.magnifyPlus
		onTriggered: setPixelSize(Qaterial.Style.userPixelSize+1)
	}

	property Action fontMinus: Action {
		shortcut: "Ctrl+-"
		text: qsTr("Csökkentés")
		icon.source: Qaterial.Icons.magnifyMinus
		onTriggered: setPixelSize(Qaterial.Style.userPixelSize-1)
	}

	property Action fontNormal: Action {
		shortcut: "Ctrl+0"
		text: qsTr("Visszaállítás")
		icon.source: Qaterial.Icons.magnifyRemoveOutline
		onTriggered: setPixelSize(Qaterial.Style.defaultPixelSize)
	}


	property Action actionFullScreen: Action {
		shortcut: "Ctrl+F11"
		text: qsTr("Teljes képernyő")
		icon.source: Client.fullScreenHelper ? Qaterial.Icons.fullscreenExit : Qaterial.Icons.fullscreen
		checked: Client.fullScreenHelper
		checkable: true

		onTriggered: Client.fullScreenHelper = checked
	}

	Connections {
		target: Client.updater

		function onGitHubUpdateCheckFailed() {
			Client.snack(qsTr("Frissítéskeresés sikertelen"))
		}

		function onGitHubUpdateAvailable(data) {
			if (data.installer !== undefined) {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									Client.updater.updateGitHub(data.installer, data.sha1)
								},
								text: qsTr("Az applikációhoz új frissítés érhető el: %1. Letöltsem?").arg(data.version),
								title: qsTr("Frissítés"),
								iconSource: Qaterial.Icons.applicationCog
							})
			} else if (Qt.platform.os == "android" || Qt.platform.os == "ios") {
				JS.questionDialog(
							{
								onAccepted: function()
								{
									Client.updater.updateNative()
								},
								text: qsTr("Az applikációhoz új frissítés érhető el: %1. Frissítsem?").arg(data.version),
								title: qsTr("Frissítés"),
								iconSource: Qaterial.Icons.applicationCog
							})
			} else {
				Client.messageInfo(qsTr("Új verzió elérhető: ")+data.version, qsTr("Frissítés"))
			}
		}

		function onAppImageUpdateFailed(errorString) {
			Client.messageError(errorString, qsTr("AppImage frissítés sikertelen"))
		}

		function onAppImageUpdateSuccess() {
			Qaterial.DialogManager.showDialog({
												  iconColor: Qaterial.Colors.green500,
												  iconSource: Qaterial.Icons.update,
												  textColor: Qaterial.Colors.green500,
												  iconFill: false,
												  iconSize: Qaterial.Style.roundIcon.size,
												  standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
												  text: qsTr("Sikeres frissítés, zárd be az alkalmazást"),
												  title: qsTr("Applikáció frissítve"),
												  onAccepted: function() {
													  Qt.quit()
												  }
											  })

		}

		function onAppImageUpdateAvailable() {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.updater.updateAppImage()
							},
							onRejected: function()
							{
								Client.updater.autoUpdate = false
								Client.messageInfo(qsTr("Automatikus frissítéskeresés letiltve, a beállításoknál tudod visszakapcsolni."),
												   qsTr("Frissítéskeresés"))
							},
							text: qsTr("Az applikációhoz új frissítés érhető el. Frissítsem?"),
							title: qsTr("Call of Suli - AppImage"),
							iconSource: Qaterial.Icons.applicationCog
						})
		}

		function onUpdateReady() {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.updater.updateExecute()
							},
							text: qsTr("A telepítő letöltődött, elindítsam?"),
							title: qsTr("Frissítés telepítése"),
							iconSource: Qaterial.Icons.checkBold
						})

		}

		function onUpdateDownloadFailed() {
			Client.messageError(qsTr("A telepítő letöltése nem sikerült"), qsTr("Frissítés sikertelen"))
		}

		function onUpdateExecuteFailed() {
			Client.messageError(qsTr("Nem sikerült elindítani a telepítőt"), qsTr("Frissítés sikertelen"))
		}

		function onUpdateExecuteStarted() {
			Qaterial.DialogManager.showDialog({
												  iconColor: Qaterial.Colors.green500,
												  iconSource: Qaterial.Icons.update,
												  textColor: Qaterial.Colors.green500,
												  iconFill: false,
												  iconSize: Qaterial.Style.roundIcon.size,
												  standardButtons: DialogButtonBox.Cancel | DialogButtonBox.Ok,
												  text: qsTr("A telepítő elindult, zárd be az alkalmazást"),
												  title: qsTr("Frissítés telepítése"),
												  onAccepted: function() {
													  Qt.quit()
												  }
											  })

		}

		function onUpdateNotAvailable() {
			Client.messageInfo(qsTr("Az applikáció naprakész"), qsTr("Nincsen új frissítés"))
		}

	}


	Connections {
		target: Client

		ignoreUnknownSignals: true

		function onServerSetAutoConnectRequest(server) {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.serverSetAutoConnect(server)
							},
							text: qsTr("Szeretnél ehhez a szerverhez az applikáció indulásakor automatikusan csatlakozni?"),
							title: server.serverName,
							iconSource: Qaterial.Icons.connection
						})
		}


	}


	Connections {
		target: Client.httpConnection

		function onPendingSslErrors(list) {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								Client.httpConnection.acceptPendingSslErrors()
							},
							text: qsTr("A szerver tanúsítványa hibás. Ennek ellenére csatlakozol hozzá?\n")+list.join("\n"),
							title: qsTr("SSL tanúsítvány"),
							iconSource: Qaterial.Icons.shieldAlertOutline
						})

		}
	}


	onWidthChanged: {
		if (width >= 992 * Qaterial.Style.pixelSizeRatio)
			Qaterial.Style.maxContainerSize = 970 * Qaterial.Style.pixelSizeRatio
		else if (width >= 768 * Qaterial.Style.pixelSizeRatio)
			Qaterial.Style.maxContainerSize = 750 * Qaterial.Style.pixelSizeRatio
		else
			Qaterial.Style.maxContainerSize = width

	}

	onClosing: close => {
		if (Client.closeWindow()) {
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
						standardButtons: DialogButtonBox.Ok
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
						standardButtons: DialogButtonBox.No | DialogButtonBox.Yes
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
		if (newSize >= Qaterial.Style.defaultPixelSize/2.5 && newSize <= Qaterial.Style.defaultPixelSize * 3.0) {
			Qaterial.Style.userPixelSize = newSize
			Client.Utils.settingsSet("window/fontSize", newSize)
		}

	}

}

