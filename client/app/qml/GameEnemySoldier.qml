import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import "Style"

GameEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height

	z: 9

	entityPrivate: ep

	property bool showPickable: false
	property bool showTarget: false

	glowColor: showPickable ? CosStyle.colorGlowItem : CosStyle.colorGlowEnemy
	glowEnabled: ep.aimedByPlayer || showPickable || showTarget

	hpColor: CosStyle.colorWarningLighter
	hpVisible: ep.aimedByPlayer
	hpValue: ep.hp

	GameEnemySoldierPrivate {
		id: ep

		onKilled: {
			spriteSequence.jumpTo("dead")
		}

		onMovingChanged: setSprite()
		onAtBoundChanged: setSprite()
		onPlayerChanged: {
			setSprite()

			if (ep.player) {
				var o = markerComponent.createObject(root)
				o.playerItem = ep.player.parentEntity
			}
		}

		onAttack: {
			spriteSequence.jumpTo("shot")
			cosClient.playSound(shotSoundFile, CosSound.EnemyShoot)
		}

		onRayCastPerformed: {
			if (cosGame.gameScene.debug)
				setray(rect)
		}

		Connections {
			target: ep.cosGame ? ep.cosGame.gameScene : null
			function onShowPickablesChanged() {
				if (ep.cosGame.gameScene.showPickables && ep.enemyData.pickableType != GameEnemyData.PickableInvalid)
					showPickable = true
				else
					showPickable = false
			}

			function onShowTargetsChanged() {
				if (ep.cosGame.gameScene.showTargets && ep.enemyData.targetId != -1)
					showTarget = true
				else
					showTarget = false
			}
		}
	}

	Component {
		id: markerComponent

		GameEnemyMarker {
			enemyPrivate: ep
		}
	}



	function setray(rect) {
		var k = mapFromItem(scene, rect.x, rect.y)

		rayRect.x = k.x
		rayRect.y = k.y
		rayRect.width = rect.width
		rayRect.height = Math.max(rect.height, 1)
		rayRect.visible = true
		timerOff.start()

	}

	Rectangle {
		id: rayRect
		color: "blue"
		visible: false
		border.width: 1
		border.color: "blue"

		Timer {
			id: timerOff
			interval: 200
			triggeredOnStart: false
			running: false
			repeat: false
			onTriggered: rayRect.visible = false
		}
	}



	function setSprite() {
		if (!ep.isAlive)
			return

		if (ep.player) {
			spriteSequence.jumpTo("idle")
		} else {
			if (!ep.atBound && ep.moving) {
				spriteSequence.jumpTo("walk")
			}
			else {
				spriteSequence.jumpTo("idle")
			}
		}
	}

}
