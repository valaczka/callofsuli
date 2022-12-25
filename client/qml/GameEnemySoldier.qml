import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameEnemySoldierImpl {
	id: control

	glowColor: (scene && scene.showObjects) ? Qaterial.Style.colorItemGlow : Qaterial.Style.colorEnemyGlow
	overlayColor: Qaterial.Style.colorEnemyGlow
	glowEnabled: aimedByPlayer || (scene && ((scene.showEnemies && hasQuestion) || (scene.showObjects && hasPickable)))

	hpProgressColor: Qaterial.Style.colorEnemyGlow
	hpProgressEnabled: aimedByPlayer

	z: 9


	onPlayerChanged: markerComponent.createObject(control)


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

