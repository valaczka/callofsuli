import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QPage {
	id: root

	title: qsTr("Conquest ") +
		   (game ? ((game.hostMode == ConquestGame.ModeHost ? "Host" : "Guest")+ " - " + game.engineId)
				 : "???")

	property ConquestGame game: null
	//property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")
	//property string closeQuestion: qsTr("Biztosan megszakítod a játékot?")
	property var onPageClose: function() { if (game) game.gameAbort() }

	readonly property int stackViewIndex: StackView.index

	property bool _mapVisible: false

	QScrollable {
		anchors.fill: parent
		visible: !_mapVisible

		Column {
			id: col
			width: Math.min(parent.width, Qaterial.Style.maxContainerSize) //parent.width-(Math.max(Client.safeMarginLeft, Client.safeMarginRight, 10)*2)
			anchors.horizontalCenter: parent.horizontalCenter
			spacing: 10

			Item {
				width: parent.width
				height: 20
			}

			CosImage {
				id: cosImage
				anchors.horizontalCenter: parent.horizontalCenter
				width: Math.min(parent.width*0.7, 800)
				glow: false
			}

			Qaterial.LabelBody2 {
				id: labelVersion
				horizontalAlignment: Qt.AlignHCenter
				wrapMode: Text.Wrap
				width: parent.width
				color: Qaterial.Style.primaryTextColor()

				padding: 20

				text: qsTr("State: ")+game.config.gameState+" - "+game.config.world
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "CONNECT"
				enabled: game
				onClicked: {
					let c = game.config
					game.sendWebSocketMessage({
												  cmd: "connect",
												  map: c.mapUuid,
												  mission: c.missionUuid,
												  level: c.missionLevel
											  })
				}
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "ENROLL "+(game ? game.playerId : "")
				enabled: game && game.playerId === -1
				onClicked: game.sendWebSocketMessage({"cmd": "enroll", "engine": game.engineId})
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "START"
				enabled: game && game.hostMode == ConquestGame.ModeHost
				onClicked: game.sendWebSocketMessage({"cmd": "start", "engine": game.engineId})
			}
		}
	}

	ConquestScene {
		id: _scene
		visible: _mapVisible
		anchors.fill: parent
		game: root.game
	}

	QButton {
		anchors.right: parent.right
		anchors.top: parent.top
		text: "PREPARED"
		enabled: game.config.gameState == ConquestConfig.StatePrepare
		onClicked: game.sendWebSocketMessage({"cmd": "prepare", "engine": game.engineId, "ready": true})
	}


	Connections {
		target: game

		function onConfigChanged() {
			let c = game.config
			console.info("*****", c.gameState, c.world)

			if (c.world != "" && (c.gameState == ConquestConfig.StatePrepare || c.gameState == ConquestConfig.StatePlay)) {
				console.warn("load", c.world)
				_mapVisible = true
				_scene.world = c.world
			}
		}
	}
}
