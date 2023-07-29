import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GamePlayerImpl {
	id: control

	z: 10

	glowColor: Qaterial.Style.colorGlow
	overlayColor: "#eeeeee"
	hpProgressColor: shield ? Qaterial.Colors.green400 : Qaterial.Colors.red400
	hpProgressEnabled: enemy || hpVisibleTimer.running

	onUnderAttack: hpVisibleTimer.restart()

	overlayEnabled: invisible
	opacity: invisible ? 0.5 : 1.0


	onRayCastPerformed: Qt.callLater(() => ray.show(rect, scene))

	GameEntityRay {
		id: ray
		color: "orange"
	}

	Timer {
		id: hpVisibleTimer
		interval: 1000
		running: false
		repeat: false
	}


}

