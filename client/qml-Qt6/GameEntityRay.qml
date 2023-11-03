import QtQuick
import QtQuick.Controls


Rectangle {
	id: control
	color: "blue"
	visible: control.parent.scene && control.parent.scene.debugView
	border.width: 1
	border.color: color
	opacity: 0.0


	/*Timer {
		id: timerOff
		interval: control.parent.scene ? control.parent.scene.timingTimerTimeoutMsec : 20
		triggeredOnStart: false
		running: control.visible
		repeat: false
		onTriggered: control.visible = false
	}*/

	function show(rect, scene) {
		if (!visible)
			return

		control.x = rect.x
		control.y = rect.y
		control.width = rect.width
		control.height = Math.max(rect.height, 1)
		anim.start()
	}

	SequentialAnimation {
		id: anim
		running: false

		PropertyAnimation {
			target: control
			property: "opacity"
			from: 0.0
			to: 1.0
			duration: 250
			easing.type: Easing.InQuad
		}

		PropertyAnimation {
			target: control
			property: "opacity"
			from: 1.0
			to: 0.0
			duration: 250
			easing.type: Easing.OutQuad
		}
	}


}
