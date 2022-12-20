import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GamePlayerPrivate {
	id: control

	z: 10

	glowColor: Qaterial.Style.colorGlow
	overlayColor: "#eeeeee"
	hpProgressColor: Qaterial.Colors.green400
	hpProgressEnabled: enemy || hpVisibleTimer.running

	onUnderAttack: hpVisibleTimer.restart()

	//overlayEnabled: ep.invisible
	//opacity: ep.invisible ? 0.5 : 1.0


	/*hpColor: ep.shield ? CosStyle.colorOK : CosStyle.colorErrorLighter
	hpVisible: ep.enemy || hpVisibleTimer.running
	hpValue: ep.shield ? ep.shield : ep.hp*/



	onRayCastPerformed: ray.show(rect, scene)


	GameEntityRay {
		id: ray
		color: "orange"
		visible: true
	}

	Timer {
		id: hpVisibleTimer
		interval: 1000
		running: false
		repeat: false
	}


}

