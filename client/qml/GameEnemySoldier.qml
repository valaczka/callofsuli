import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameEnemySoldierPrivate {
	id: control

	glowColor: Qaterial.Style.colorEnemyGlow
	overlayColor: Qaterial.Style.colorEnemyGlow
	glowEnabled: aimedByPlayer || (scene && scene.showEnemies)

	hpProgressColor: Qaterial.Style.colorEnemyGlow
	hpProgressEnabled: aimedByPlayer

	z: 9

	/*property bool showPickable: false
	property bool showTarget: false

	glowColor: showPickable ? CosStyle.colorGlowItem : CosStyle.colorGlowEnemy
	glowEnabled: ep.aimedByPlayer || showPickable || showTarget

	hpVisible: ep.aimedByPlayer
	hpValue: ep.hp*/


	onPlayerChanged: markerComponent.createObject(control)


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

	Component {
		id: markerComponent

		GameEnemyMarker {
			enemy: control
		}
	}


	onRayCastPerformed: ray.show(rect, scene)

	GameEntityRay {
		id: ray
		color: "blue"
	}



}

