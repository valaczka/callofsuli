import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


GameButton {
	id: root
	size: 60

	tap.enabled: false

	property bool _animationEnabled: true
	property real dstX: 0
	property real dstY: 0

	onXChanged: dstX = x
	onYChanged: dstY = y

	Behavior on x {
		enabled: _animationEnabled
		NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
	}

	Behavior on y {
		enabled: _animationEnabled
		NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
	}

	Timer {
		running: true
		interval: 200
		repeat: true
		onTriggered: {
			root.x = Math.max(0, Math.min(dstX, root.parent.width-root.width))
			root.y = Math.max(0, Math.min(dstY, root.parent.height-root.height))
		}
	}

	MultiPointTouchArea {
		anchors.fill: parent
		maximumTouchPoints: 1

		property bool hasTouch: false

		touchPoints: [
			TouchPoint {
				id: point
			}
		]

		onTouchUpdated: touchPoints => {
							if (touchPoints.length) {
								if (!hasTouch) {
									root.clicked()
									tapAnim.start()
								}

								hasTouch = true

								let dx = touchPoints[0].x - root.width*0.5
								let dy = touchPoints[0].y - root.height*0.5

								if (dx*dx + dy*dy > 0.25*root.size*root.size) {
									dstX = root.x + dx
									dstY = root.y + dy
								}
							} else {
								hasTouch = false
							}
						}
	}


	function reset() {
		let padding = Math.max(10, Client.safeMarginRight, Client.safeMarginBottom,
							   (Qt.platform.os == "android" || Qt.platform.os == "ios") ? 30 : 0)

		_animationEnabled = false

		x = parent.width - padding - size
		y = parent.height - padding	- size

		_animationEnabled = true
	}

	Component.onCompleted: reset()
}
