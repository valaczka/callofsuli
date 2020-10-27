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

	width: scenePrivate.implicitWidth
	height: scenePrivate.implicitHeight

	property alias scenePrivate: scenePrivate

	y: game ? Math.max(0, game.height-height) : 0

	GameScenePrivate {
		id: scenePrivate

		onLayersLoaded: {
			console.debug("Layers loaded")

			createLadders()

			if (!game.player) {
				var p = playerComponent.createObject(scene)
				p.onXChanged.connect(setXOffset)
				p.onYChanged.connect(setYOffset)
				p.loadSprites()
				game.player = p
			}
		}
	}

	viewport: Viewport {
		id: vp
		width: scene.game ? scene.game.width : scene.width
		height: scene.game ? scene.game.height : scene.height
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
			collidesWith: Box.Category1
			readonly property bool baseGround: true
		}
	}

	Component {
		id: playerComponent

		GamePlayer { }
	}



	Keys.onPressed: {
		scene.forceActiveFocus()
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
			case Qt.Key_F3:
				loadEnemies()
				break;
			}
		}

		event.accepted = true
	}




	Connections {
		target: parent
		onWidthChanged: setXOffset()
		onHeightChanged: setYOffset()
	}


	Component {
		id: ladderComponent

		GameLadder { }
	}


	function createLadders() {
		if (!game || !game.ladders)
			return

		for (var i=0; i<game.ladders.length; i++) {
			var l = game.ladders[i]
			console.debug("ladder", i, l, l.boundRect)

			var obj = ladderComponent.createObject(scene,{
													   ladder: l
												   })
		}
	}


	function setXOffset() {
		if (!game.width || !game.player)
			return

		if (game.player.facingLeft && (game.player.x-vp.xOffset < 500))
			vp.xOffset = game.player.x-500
		else if (!game.player.facingLeft && (game.player.x-(vp.xOffset+game.width)+500) > 0)
			vp.xOffset = game.player.x - game.width + 500
	}

	function setYOffset() {
		if (!game.height || !game.player)
			return

		// TODO //
	}


}
