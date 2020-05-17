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

		onGamePrepared: {
			if (game.gamePlayMode == Game.GamePlayOffline)
				game.start()
		}

		onGameRegistered: {
			game.start()
		}

		onGameStarted: {
			console.debug("STARTED")
		}

		onGameSucceed: {
			console.debug("MISSION COMPLETED")
			game.finish()
		}

		onIntroPopulated:  {
			console.debug("INTRO", intro)
			var o = JS.createPage("Intro", {"intro": intro}, page)
			o.pagePopulated.connect(function() {
				pageGame._hasIntro = true
			})
		}

		onOutroPopulated:  {
			console.debug("OUTRO", outro)
			var o = JS.createPage("Intro", {"intro": outro}, page)
			o.pagePopulated.connect(function() {
				pageGame._hasOutro = true
			})
		}

		onTargetPopulated:  {
			console.debug("TARGET", module, task)
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
		source: "qrc:/img/villa.png"
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

	Label {
		id: ttt
		anchors.centerIn: parent
		text: "loaded "+game.targetBlockDone+"+("+game.targetBlockDone+"/"+game.targetCount+") / "+game.targetCount
		visible: false
	}

	Label {
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

		text: qsTr("done")

		visible: game.gameState == Game.GameRun

		onClicked: {
			game.check({index: 0})
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
				script: cosClient.sendMessageInfo(qsTr("Játék"), qsTr("START"))
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
			game.introDone()
		} else if (_hasOutro) {
			game.outroDone()
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
		} else if (game.gameState == Game.GamePrepared || game.gameState == Game.GameClosing) {
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
		} else if (game.gameState == Game.GamePrepared || game.gameState == Game.GameClosing) {
			game.abort()
			return true
		}

		return false
	}
}
