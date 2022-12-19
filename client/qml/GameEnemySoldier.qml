import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0


GameEnemySoldierPrivate {
	id: control

	glowColor: Client.Style.colorEnemyGlow
	overlayColor: Client.Style.colorEnemyGlow
	hpProgressColor: Client.Style.colorEnemyGlow
	hpProgressEnabled: true

	z: 9

	/*property bool showPickable: false
	property bool showTarget: false

	glowColor: showPickable ? CosStyle.colorGlowItem : CosStyle.colorGlowEnemy
	glowEnabled: ep.aimedByPlayer || showPickable || showTarget

	hpVisible: ep.aimedByPlayer
	hpValue: ep.hp*/


	/*onMovingChanged: setSprite()
		onAtBoundChanged: setSprite()
		onPlayerChanged: {
			setSprite()

			if (ep.player) {
				var o = markerComponent.createObject(root)
				o.playerItem = ep.player.parentEntity
			}
		}

		*/


	/*Connections {
			target: ep.cosGame ? ep.cosGame.gameScene : null
			function onShowPickablesChanged() {
				if (ep.cosGame.gameScene.showPickables && ep.enemyData.pickableType !== GamePickablePrivate.PickableInvalid)
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

			function onIsSceneZoomChanged() {
				overlayEnabled = ep.cosGame.gameScene.isSceneZoom
			}
		}*/

	/*Component {
		id: markerComponent

		GameEnemyMarker {
			enemyPrivate: ep
		}
	}*/


	onRayCastPerformed: {
		if (scene && scene.debugView) {
			var k = mapFromItem(scene, rect.x, rect.y)
			rayRect.x = k.x
			rayRect.y = k.y
			rayRect.width = rect.width
			rayRect.height = Math.max(rect.height, 1)
			rayRect.visible = true
			timerOff.start()
		}

	}

	Rectangle {
		id: rayRect
		color: "blue"
		visible: false
		border.width: 1
		border.color: "blue"

		Timer {
			id: timerOff
			interval: scene ? scene.timingTimerTimeoutMsec : 20
			triggeredOnStart: false
			running: false
			repeat: false
			onTriggered: rayRect.visible = false
		}
	}



}

