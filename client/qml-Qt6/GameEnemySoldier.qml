import QtQuick
import QtQuick.Controls
import CallOfSuli
import Box2D 2.0
import Qaterial as Qaterial
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


	onRayCastPerformed: rect => {if (ray) ray.show(rect, scene)}

	GameEntityRay {
		id: ray
		color: "blue"
	}

}

