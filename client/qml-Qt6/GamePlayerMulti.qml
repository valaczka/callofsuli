import QtQuick
import QtQuick.Controls
import CallOfSuli
import Box2D 2.0
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


GamePlayerMultiImpl {
	id: control

	z: 10

	glowColor: Qaterial.Style.colorGlow
	overlayColor: isSelf ? "#eeeeee" : Qaterial.Style.colorGlow
	hpProgressColor: shield ? Qaterial.Colors.green400 : Qaterial.Colors.red400
	hpProgressEnabled: enemy || hpVisibleTimer.running

	onUnderAttack: hpVisibleTimer.restart()

	overlayEnabled: invisible
	opacity: invisible ? 0.5 : 1.0


	onRayCastPerformed: rect => {if (ray) ray.show(rect, scene)} //Qt.callLater(() => {if (ray) ray.show(rect, scene)})

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

