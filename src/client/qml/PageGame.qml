import QtQuick 2.14
import QtQuick.Controls 2.14
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: pageGame

	property alias game: game

	property bool hasWindowCloseRequest: false
	property bool _hasIntro: false
	property bool _hasOutro: false
	property bool _isFirst: true

	signal pagePopulated()

	Game {
		id: game
		client: cosClient

		onGamePrepareError: {
			var d = JS.dialogMessageWarning(qsTr("Játék"), errorString)
			d.onClosedAndDestroyed.connect(function() {
				mainStack.back()
			})
		}

		onGamePrepared: game.playPrepared()

		onGameStarted: {
			console.debug("STARTED")
		}

		onGameSucceed: {
			var d = JS.dialogMessageInfo(qsTr("Játék"), "MISSION COMPLETED")
			d.onClosedAndDestroyed.connect(function() {
				if (outro) {
					console.debug("OUTRO", outro)
					var o = JS.createPage("Intro", {"intro": outro})
					o.pagePopulated.connect(function() {
						pageGame._hasOutro = true
					})
				} else {
					game.close()
				}
			})
		}

		onGameFailed: {
			var d = JS.dialogMessageError(qsTr("Játék"), "MISSION FAILED")
			d.onClosedAndDestroyed.connect(function() {
				game.close()
			})
		}

		onIntroPopulated:  {
			console.debug("INTRO", intro)
			var o = JS.createPage("Intro", {"intro": intro})
			o.pagePopulated.connect(function() {
				pageGame._hasIntro = true
			})
		}

		onTargetPopulated:  {
			console.debug("TARGET", module, task, solution)
			if (game.showCorrect)
				buttonTrue.valasz = solution.index
		}

		onSolutionCorrect: {
			console.debug("CORRECT")
		}

		onSolutionFail: {
			console.debug("FAILED")
		}
	}

	background: Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
	}


	Item {
		id: loadingItem
		anchors.fill: parent

		BusyIndicator {
			anchors.centerIn: parent
			width: 100
			height: 100
			running: true
		}
	}

	QLabel {
		id: ttt
		anchors.centerIn: parent
		text: "loaded "+game.targetBlockDone+"+("+game.targetBlockDone+"/"+game.targetCount+") / "+game.targetCount
		visible: false
	}

	QLabel {
		anchors.top: ttt.bottom
		anchors.horizontalCenter: ttt.horizontalCenter
		text: game.currentMSec+"/"+game.maxMSec+"    "+game.currentHP+"/"+game.maxHP
	}

	QButton {
		anchors.right: parent.right
		anchors.verticalCenter: parent.verticalCenter

		text: qsTr("start")

		visible: game.gameState == Game.GamePrepared

		onClicked: {
			game.start()
		}
	}


	QButton {
		anchors.left: parent.left
		anchors.verticalCenter: parent.verticalCenter

		text: qsTr("false")

		visible: game.gameState == Game.GameRun

		onClicked: {
			game.check({index: -1})
		}
	}


	QButton {
		id: buttonTrue
		anchors.left: parent.left
		anchors.verticalCenter: parent.verticalCenter

		text: qsTr("true")

		visible: game.gameState == Game.GameRun

		property int valasz: 0

		onClicked: {
			game.check({index: valasz})
		}
	}


	states: [
		State {
			name: "GameRun"
			when: game.gameState == Game.GameRun
			PropertyChanges {
				target: loadingItem
				visible: false
			}
			PropertyChanges {
				target: ttt
				visible: true
			}
		},

		State {
			name: "GameFinished"
			when: game.gameState == Game.GameFinished
		}

	]


	transitions: [
		Transition {
			from: "*"
			to: "GameRun"
			ScriptAction {
				script: console.debug("STARTED")
			}
		},

		Transition {
			from: "*"
			to: "GameFinished"
			ScriptAction {
				script: {
					if (game.gameResult == Game.GameResultAborted) {
						var d = JS.dialogMessageWarning(qsTr("Játék"), qsTr("A játék megszakadt"), "")
						d.onClosedAndDestroyed.connect(function() {
							if (pageGame.hasWindowCloseRequest)
								mainWindow.close()
							else
								mainStack.back()
						})
					} else {
						d = JS.dialogMessageInfo(qsTr("Játék"), qsTr("A játék véget ért"), "")
						d.onClosedAndDestroyed.connect(function() {
							mainStack.back()
						})
					}
				}
			}
		}
	]

	StackView.onRemoved: destroy()

	StackView.onActivated: {
		if (_isFirst) {
			pagePopulated()
			_isFirst = false
		} else if (_hasIntro) {
			_hasIntro = false
			game.start()
		} else if (_hasOutro) {
			_hasOutro = false
			game.close()
		}
	}



	function windowClose() {
		if (game.gameState == Game.GameOpening || game.gameState == Game.GameRegistered || game.gameState == Game.GameRun) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan megszakítod a játékot?")})
			d.accepted.connect(function() {
				pageGame.hasWindowCloseRequest = true
				game.abort()
			})
			d.open()
			return false
		} else if (game.gameState == Game.GamePrepared) {
			pageGame.hasWindowCloseRequest = true
			game.abort()
			return false
		}

		return true
	}


	function stackBack() {
		if (mainStack.depth > pageGame.StackView.index+1) {
			if (!mainStack.get(pageGame.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageGame.StackView.index+1) {
					mainStack.pop(pageGame)
				}
			}
			return true
		}

		if (game.gameState == Game.GameOpening || game.gameState == Game.GameRegistered || game.gameState == Game.GameRun) {
			var d = JS.dialogCreateQml("YesNo", {title: qsTr("Biztosan megszakítod a játékot?")})
			d.accepted.connect(function() {
				game.abort()
			})
			d.open()
			return true
		} else if (game.gameState == Game.GamePrepared) {
			game.abort()
			return true
		}

		return false
	}
}
