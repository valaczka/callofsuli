import QtQuick 2.12
import QtQuick.Controls 2.12
import Bacon2D 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Scene {
	id: scene
	debug: true
	physics: true
	focus: true

	width: scenePrivate.implicitWidth
	height: scenePrivate.implicitHeight

	property alias scenePrivate: scenePrivate



	GameScenePrivate {
		id: scenePrivate

		onSceneLoaded: {
			game.currentScene = gameScene
			createLadders()
			game.startGame()
			scene.forceActiveFocus()
		}
	}

	onRunningChanged: console.info("SCENE run ", running)

	PhysicsEntity {
		x: 0
		y: scene.height+5
		width: scene.width
		height: 10

		fixtures: Box {
			width: scene.width
			height: 10
			density: 1
			restitution: 0
			friction: 1
			categories: Box.Category1
			collidesWith: (Box.Category2|Box.Category5)
			readonly property bool baseGround: true
		}
	}


	MouseArea {
		id: area
		anchors.fill: parent
		hoverEnabled: true
	}

	Keys.onBackPressed: mainStack.back()

	Keys.onPressed: {
		if (game.player) {
			switch(event.key) {
			case Qt.Key_Left:
				if (event.modifiers & Qt.ShiftModifier)
					game.player.runLeft()
				else
					game.player.walkLeft()
				break;
			case Qt.Key_Right:
				if (event.modifiers & Qt.ShiftModifier)
					game.player.runRight()
				else
					game.player.walkRight()
				break;
			case Qt.Key_Up:
				game.player.moveUp()
				break;
			case Qt.Key_Down:
				game.player.moveDown()
				break;
			case Qt.Key_Space:
				game.player.entityPrivate.attackByGun()
				break
			}
		}

		event.accepted = true
	}

	Keys.onReleased: {
		if (game.player) {
			switch(event.key) {
			case Qt.Key_Left:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Right:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Up:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_Down:
				if(!event.isAutoRepeat)
					game.player.stopMoving();
				break;
			case Qt.Key_F5:
				game.loadQuestion()
				break;
			case Qt.Key_X:
				if (event.modifiers & Qt.ShiftModifier && area.containsMouse && game.player) {
					game.player.x = area.mouseX
					game.player.y = area.mouseY
					game.player.entityPrivate.ladderClimbFinish()
				}
				break;
			}
		}

		event.accepted = true
	}



	Component {
		id: playerComponent

		GamePlayer { }
	}


	Component {
		id: ladderComponent

		GameLadder { }
	}

	Component {
		id: enemySoldierComponent

		GameEnemySoldier { }
	}

	Component {
		id: questionComponent

		GameQuestion { }
	}


	function createPlayer() : Item {
	if (!game.player) {
		var p = playerComponent.createObject(scene)
		p.loadSprites()
		return p
	}
		return null
	}


	function createComponent(enemyType: int) : Item {
		var obj = null

		switch (enemyType) {
			case GameEnemyData.EnemySoldier:
				obj = enemySoldierComponent.createObject(scene)
			break
			case GameEnemyData.EnemyOther:
				obj = enemySoldierComponent.createObject(scene)
			break
		}

		return obj
	}


	function createLadders() {
		if (!game || !game.ladderCount)
		return

		for (var i=0; i<game.ladderCount; i++) {
			var l = game.ladderAt(i)

			var obj = ladderComponent.createObject(scene,{
				ladder: l
			})
		}
	}


	function createQuestion(questionData: json) : Item {
		var obj = questionComponent.createObject(game.itemPage,{
			//ladder: l
		})

		return obj
	}
}

