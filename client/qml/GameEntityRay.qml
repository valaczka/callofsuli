import QtQuick 2.15
import QtQuick.Controls 2.15


Rectangle {
	id: control
	color: "blue"
	visible: false
	border.width: 1
	border.color: color

	Timer {
		id: timerOff
		interval: control.parent.scene ? control.parent.scene.timingTimerTimeoutMsec : 20
		triggeredOnStart: false
		running: control.visible
		repeat: false
		onTriggered: control.visible = false
	}

	function show(rect, scene) {
		if (scene && scene.debugView) {
			control.x = rect.x
			control.y = rect.y
			control.width = rect.width
			control.height = Math.max(rect.height, 1)
			control.visible = true
		}
	}


}
