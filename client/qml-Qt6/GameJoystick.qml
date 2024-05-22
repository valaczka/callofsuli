import QtQuick
import Qaterial as Qaterial

Item {
	id: root
	width: size
	height: size

	property real size: 120 * Qaterial.Style.pixelSizeRatio
	property real thumbSize: 40 * Qaterial.Style.pixelSizeRatio

	property real currentX: 0.0
	property real currentY: 0.0
	property real currentAngle: 0.0
	property real currentDistance: 0.0
	property bool hasTouch: false

	readonly property real _circleRadius: (Math.min(width, height)-thumbSize)/2

	signal joystickMoved(real x, real y)
	signal directionChanged(real angle, real distance)

	onWidthChanged: moveThumb(root.width/2, root.height/2)
	onHeightChanged: moveThumb(root.width/2, root.height/2)
	onXChanged: _translate.dstX = x
	onYChanged: _translate.dstY = y

	transform: Translate {
		id: _translate

		property real dstX: 0
		property real dstY: 0

		Behavior on x {
			NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
		}

		Behavior on y {
			NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
		}
	}

	Timer {
		running: true
		interval: 400
		repeat: true
		onTriggered: {
			_translate.x = Math.max(-root.x, Math.min(_translate.dstX-root.x, root.parent.width-root.x-root.width))
			_translate.y = Math.max(-root.y, Math.min(_translate.dstY-root.y, root.parent.height-root.y-root.height))
		}
	}


	Rectangle {
		width: _circleRadius*2
		height: _circleRadius*2
		anchors.centerIn: parent
		radius: _circleRadius

		color: "transparent"
		border.color: "white"
		border.width: 3
		opacity: root.enabled ? 0.5	 : 0.2
	}

	Rectangle {
		id: thumb
		width: thumbSize
		height: thumbSize
		radius: thumbSize/2
		color: "white"

		x: (root.width-width)/2
		y: (root.height-height)/2

		border.color: "black"
		border.width: 2

		opacity: root.enabled ? (hasTouch ? 1.0 : 0.6) : 0.3

		Behavior on x {
			NumberAnimation { duration: 200; easing.type: Easing.OutSine }
		}

		Behavior on y {
			NumberAnimation { duration: 200; easing.type: Easing.OutSine }
		}
	}



	MultiPointTouchArea {
		anchors.fill: parent
		maximumTouchPoints: 1

		touchPoints: [
			TouchPoint {
				id: point
			}
		]

		onTouchUpdated: touchPoints => {
							if (touchPoints.length) {
								hasTouch = true
								moveThumb(touchPoints[0].x, touchPoints[0].y)
							} else {
								hasTouch = false
								moveThumb(root.width/2, root.height/2)
								currentX = 0.0
								currentY = 0.0
								currentDistance = 0.0

								joystickMoved(currentX, currentY)
								directionChanged(currentAngle, currentDistance)
							}

						}
	}


	function moveThumb(centerX, centerY) {
		let dx = centerX/(root.width*0.5) - 1.
		let dy = centerY/(root.height*0.5) - 1.

		let angle = Math.atan2(-dy, dx)
		let distance = Math.sqrt(dx*dx + dy*dy)

		let s = Math.min(1., distance) * _circleRadius

		thumb.x = root.width*0.5 + s*Math.cos(angle) -thumb.width/2
		thumb.y = root.height*0.5 - s*Math.sin(angle) -thumb.height/2

		if (!hasTouch)
			return

		if (distance > 1) {
			_translate.dstX = root.x + _translate.x + (distance-1.) * _circleRadius * Math.cos(angle)
			_translate.dstY = root.y + _translate.y - (distance-1.) * _circleRadius * Math.sin(angle)
		}

		currentX = dx
		currentY = dy
		currentAngle = angle
		currentDistance = distance

		joystickMoved(currentX, currentY)
		directionChanged(currentAngle, currentDistance)
	}

	function moveThumbRelative(dx, dy) {
		moveThumb(root.width*dx, root.height*dy)
	}
}
