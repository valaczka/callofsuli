import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "JScript.js" as JS

QPage {
	id: control

	readonly property bool _live: Qt.platform.os == "wasm" ? false : true
	property int panelId: -1

	closeQuestion: qsTr("Biztosan lezárod a kapcsolatot a szerverrel?")

	stackPopFunction: function() {
		if (!appBar.visible) {
			appBar.visible = true
			return false
		}

		return true
	}

	onPageClose: function() {
		if (Client.server && Client.server.user.loginState == User.LoggedIn)
			Client.webSocket.close()
	}

	title: Client.server ? Client.server.serverName : ""
	subtitle: panelId > 0 ? qsTr("Infopanel #%1").arg(panelId) : ""

	appBar.backButtonVisible: true
	appBar.rightPadding: Qaterial.Style.horizontalPadding
	appBar.rightComponent: Row {
		Qaterial.AppBarButton {
			action: actionFullscreen
			anchors.verticalCenter: parent.verticalCenter
		}
		Qaterial.AppBarButton {
			visible: !Client.eventStream || !_live
			action: actionReload
			anchors.verticalCenter: parent.verticalCenter
		}
		Qaterial.AppBarButton {
			action: actionLogout
			anchors.verticalCenter: parent.verticalCenter
		}
	}

	onPanelIdChanged: actionReload.trigger()



	Qaterial.StackView
	{
		id: panelStackView
		anchors.fill: parent

		initialItem: Item {
			CosImage {
				width: Math.min(panelStackView.width*0.7, 800)
				anchors.centerIn: parent
				glowRadius: 2
			}

			/*Image {
				anchors.centerIn: parent

				//source: _url.length ? "image://qrcode/"+Qt.btoa(_url) : ""
				source: "image://qrcode/"+Qt.btoa("https://localhost:10101?page=registration&oauth=google&code=OIUDFF")

				width: Math.min(parent.width*0.8, parent.height*0.7)
				height: Math.min(parent.width*0.8, parent.height*0.7)

				fillMode: Image.PreserveAspectFit

				sourceSize.width: 500
				sourceSize.height: 500
			}*/

		}
	}

	Component {
		id: cmpLabel

		Qaterial.LabelHeadline5 {  }
	}

	Action {
		id: actionReload
		icon.source: Qaterial.Icons.reload

		onTriggered: {
			if (panelId > 0) {
				if (!_live) {
					Client.send(WebSocket.ApiPanel, panelId)
					.done(control, function(r){ reload(r) })
					.fail(control, JS.failMessage("Letöltés sikertelen"))
					return
				}

				if (Client.eventStream) {
					console.warn("Event stream already exists")
					return
				}
				Client.eventStream = Client.webSocket.getEventStream(WebSocket.ApiPanel, panelId+"/live")
			} else {
				Client.send(WebSocket.ApiPanel, "")
				.done(control, function(r){
					panelId = r.id
				})
				.fail(control, JS.failMessage("Letöltés sikertelen"))
			}
		}
	}

	Timer {
		interval: 5000
		triggeredOnStart: true
		repeat: true
		running: !_live && panelId > 0
		onTriggered: actionReload.trigger()
	}

	Action {
		id: actionFullscreen
		icon.source: control.appBar.visible ? Qaterial.Icons.fullscreen : Qaterial.Icons.fullscreenExit
		onTriggered: control.appBar.visible = !control.appBar.visible
	}

	Action {
		id: actionLogout
		icon.source: Qaterial.Icons.logoutVariant
		onTriggered: Client.logout()
	}

	Connections {
		target: Client.eventStream

		function onEventJsonReceived(event, json) {
			console.debug("EVENT", event, json)
			reload(json)
		}
	}

	StackView.onActivated: {
		if (Client.server && (Client.server.user.roles & Credential.Panel)) {
			actionReload.trigger()
		}
	}

	Component.onDestruction: {
		Client.eventStream = null
	}


	function reload(json) {
		console.debug("RELOAD", json)
		panelStackView.replace(cmpLabel, {text: JSON.stringify(json)})
	}


}
