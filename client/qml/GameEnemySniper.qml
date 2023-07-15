import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameEnemySniperImpl {
	id: control

	glowColor: (scene && scene.showObjects) ? Qaterial.Style.colorItemGlow : Qaterial.Style.colorEnemyGlow
	overlayColor: Qaterial.Style.colorEnemyGlow
	glowEnabled: aimedByPlayer || (scene && (scene.showEnemies || (scene.showObjects && hasPickable)))

	hpProgressColor: Qaterial.Style.colorEnemyGlow
	hpProgressEnabled: aimedByPlayer

	z: 9

	property bool playerDiscovered: false

	Component {
		id: crosshairComponent

		GameEnemySniperCrosshair {
			sniper: control
			z: 16
		}
	}


	onPlayerChanged: {
		if (player) {
			playerDiscovered = true
			var o = crosshairComponent.createObject(control.scene)
		}
	}


	onRayCastPerformed: {
		rect.width -= bodyRect.width
		if (!facingLeft)
			rect.x += bodyRect.width
		ray.show(rect, scene)
	}


	GameEntityRay {
		id: ray
		color: Qaterial.Colors.green500
		visible: (control.scene && control.scene.debugView) || (control.isAlive && !control.player && playerDiscovered)
	}



}

