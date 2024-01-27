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

	property bool _mapVisible: game && game.config.gameState == ConquestConfig.StatePlay

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
				text: "LIST"
				enabled: game
				onClicked: game.getEngineList()
			}

			Repeater {
				model: game ? game.engineModel : null

				delegate: QButton {
					anchors.horizontalCenter: parent.horizontalCenter
					text: "CONNECT (" + engineId + ") - " + owner
					enabled: game
					onClicked: {
						console.warn("*****", engineId)
						game.sendWebSocketMessage({
													  cmd: "connect",
													  engine: engineId
												  })
					}
				}
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "CREATE"
				enabled: game && game.engineId < 0
				onClicked: game.gameCreate()
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "ENROLL "+(game ? game.playerId : "")
				enabled: game && game.engineId > 0 && game.playerId === -1
				onClicked: game.sendWebSocketMessage({"cmd": "enroll", "engine": game.engineId})
			}

			QButton {
				anchors.horizontalCenter: parent.horizontalCenter
				text: "START"
				enabled: game && game.hostMode == ConquestGame.ModeHost && game.engineId > 0
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

	Qaterial.LabelBody2 {
		text: {
			if (!game)
				return ""

			let c = game.config

			let text="stage: " + c.currentStage + "\n"
			text += "turn: " + c.currentTurn + "\n"

			if (c.currentTurn >= 0 && c.currentTurn < c.turnList.length) {
				let ct = c.turnList[c.currentTurn]
				text += "turnPlayer: " + ct.player + "\n"
				text += "turnSubStage: " + ct.subStage + "\n"
				text += "turnSubStageEnd: " + ct.subStageEnd + "\n"
			}

			text += "q: " + JSON.stringify(c.currentQuestion) + "\n"

			return text
		}
	}

	ConquestTurnChart {
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: 50
		game: root.game
	}

	QButton {
		anchors.right: parent.right
		anchors.top: parent.top
		text: "PREPARED"
		enabled: game.config.gameState == ConquestConfig.StatePrepare
		onClicked: game.sendWebSocketMessage({"cmd": "prepare", "engine": game.engineId, "ready": true})
	}

	QButton {
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		text: "ANSWER"
		onClicked: {
			game.sendWebSocketMessage({
										  cmd: "answer",
										  engine: game.engineId,
										  answer: "hello"
									  })
		}
	}

}
