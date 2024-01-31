import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
	id: root

	property ConquestGame game: null

	property alias itemFighter1: _fighter1
	property alias itemFighter2: _fighter2

	readonly property int _answerState: game ? game.currentTurn.answerState : ConquestTurn.AnswerPending

	visible: game && game.fighter1.playerId !== -1 && game.fighter2.playerId !== -1
	radius: 5

	implicitHeight: _row.implicitHeight + 20 * Qaterial.Style.pixelSizeRatio
	implicitWidth: _row.implicitHeight + 20 * Qaterial.Style.pixelSizeRatio

	color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.5)

	Row {
		id: _row
		anchors.centerIn: parent
		spacing: 5

		Rectangle {
			id: _fighter1
			width: 120
			height: 120
			color: "white"

			Qaterial.LabelBody2 {
				id: _msec1
				color: "black"
			}
		}

		Rectangle {
			id: _fighter2
			width: 120
			height: 120
			color: "white"

			Qaterial.LabelBody2 {
				id: _msec2
				color: "black"
			}
		}

		states: [
			State {
				name: "f1"
				when: _answerState == ConquestTurn.AnswerPlayerWin

				PropertyChanges {
					target: _fighter2
					opacity: 0.3
				}
			},

			State {
				name: "f2"
				when: _answerState == ConquestTurn.AnswerPlayerLost

				PropertyChanges {
					target: _fighter1
					opacity: 0.3
				}
			}
		]

		transitions: [
			Transition {
				PropertyAnimation {
					property: "opacity"
					duration: 350
					easing.type: Easing.InOutQuad
				}
			}
		]
	}

	Connections {
		target: game

		function onCurrentTurnChanged() {
			let ms1 = 0
			let ms2 = 0

			if (game.fighter1.playerId !== -1) {
				ms1 = game.currentTurn.getElapsed(game.fighter1.playerId)
			}

			if (game.fighter2.playerId !== -1) {
				ms2 = game.currentTurn.getElapsed(game.fighter2.playerId)
			}

			_msec1.text = ms1 > 0 ? ms1 : ""
			_msec2.text = ms2 > 0 ? ms2 : ""
		}
	}
}
