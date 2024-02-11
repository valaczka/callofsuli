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

	implicitHeight: _row.implicitHeight + 80 * Qaterial.Style.pixelSizeRatio
	implicitWidth: _row.implicitWidth + 80 * Qaterial.Style.pixelSizeRatio

	color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.85)

	Row {
		id: _row
		anchors.centerIn: parent
		spacing: 25

		ConquestBattleInfoPlayer {
			id: _fighter1
			flipped: true
			character: game && game.fighter1.playerId !== -1 ? game.fighter1.character : ""

		}

		ConquestBattleInfoPlayer {
			id: _fighter2
			character: game && game.fighter2.playerId !== -1 ? game.fighter2.character : ""
		}


		states: [
			State {
				name: "f1"
				when: _answerState == ConquestTurn.AnswerPlayerWin

				PropertyChanges {
					target: _fighter2
					opacity: 0.5
				}
			},

			State {
				name: "f2"
				when: _answerState == ConquestTurn.AnswerPlayerLost

				PropertyChanges {
					target: _fighter1
					opacity: 0.5
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
			let s1 = false
			let s2 = false

			if (game.fighter1.playerId !== -1) {
				ms1 = game.currentTurn.getElapsed(game.fighter1.playerId)
				s1 = game.currentTurn.getSuccess(game.fighter1.playerId)
			}

			if (game.fighter2.playerId !== -1) {
				ms2 = game.currentTurn.getElapsed(game.fighter2.playerId)
				s2 = game.currentTurn.getSuccess(game.fighter2.playerId)
			}

			_fighter1.answerMsec = ms1
			_fighter2.answerMsec = ms2

			if (ms1 > 0) {
				_fighter1.answerState = s1 ? ConquestBattleInfoPlayer.AnswerSuccess : ConquestBattleInfoPlayer.AnswerFailed
			} else {
				_fighter1.answerState = ConquestBattleInfoPlayer.AnswerInvalid
			}

			if (ms2 > 0) {
				_fighter2.answerState = s2 ? ConquestBattleInfoPlayer.AnswerSuccess : ConquestBattleInfoPlayer.AnswerFailed
			} else {
				_fighter2.answerState = ConquestBattleInfoPlayer.AnswerInvalid
			}
		}
	}
}
