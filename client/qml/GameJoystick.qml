import QtQuick
import Qaterial as Qaterial

Item {
	id: root

	property bool extendedSize: false

	width: Math.min(extendedSize ? size*3 : size*1.3,
					maxWidth > 0 ? maxWidth : parent.width)

	height: Math.min(extendedSize ? size*3 : size*1.3,
					 maxHeight > 0 ? maxHeight : parent.height)

	property real size: 120 * Qaterial.Style.pixelSizeRatio
	property real thumbSize: 40 * Qaterial.Style.pixelSizeRatio

	property real currentX: 0.0
	property real currentY: 0.0
	property real currentAngle: 0.0
	property real currentDistance: 0.0
	property bool hasTouch: false

	property real maxHeight: 0
	property real maxWidth: 0

	readonly property real _circleRadius: (size-thumbSize)/2
	readonly property real _innerHPadding: (width-size)/2
	readonly property real _innerVPadding: (height-size)/2

	signal joystickMoved(real x, real y)
	signal directionChanged(real angle, real distance)

	onWidthChanged: moveThumb(root.width/2, root.height/2)
	onHeightChanged: moveThumb(root.width/2, root.height/2)
	onXChanged: reset()
	onYChanged: reset()

	transform: Translate {
		id: _translate

		property real dstX: 0
		property real dstY: 0

		Behavior on x {
			id: _behaviorX
			NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
		}

		Behavior on y {
			id: _behaviorY
			NumberAnimation { duration: 1200; easing.type: Easing.OutSine }
		}
	}

	Timer {
		id: _timer
		running: true
		interval: 400
		repeat: true
		onTriggered: {
			_translate.x = Math.max(-root.x-_innerHPadding,
									Math.min(_translate.dstX-root.x, root.parent.width+_innerHPadding-root.x-root.width))
			_translate.y = Math.max(-root.y-_innerVPadding,
									Math.min(_translate.dstY-root.y, root.parent.height+_innerHPadding-root.y-root.height))
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
								if (hasTouch) {
									moveThumb(touchPoints[0].x, touchPoints[0].y)
								} else {
									_behaviorX.enabled = false
									_behaviorY.enabled = false
									_translate.dstX = root.x + _translate.x + touchPoints[0].x - root.width/2
									_translate.dstY = root.y + _translate.y + touchPoints[0].y - root.height/2
									_timer.triggered()
									_behaviorX.enabled = true
									_behaviorY.enabled = true
								}

								hasTouch = true
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
		let dx = (centerX-_innerHPadding)/(size*0.5) - 1.
		let dy = (centerY-_innerVPadding)/(size*0.5) - 1.

		let angle = Math.atan2(-dy, dx)
		let distance = Math.sqrt(dx*dx + dy*dy)

		let s = Math.min(1., distance) * _circleRadius

		thumb.x = root.width*0.5 + s*Math.cos(angle) -thumb.width/2
		thumb.y = root.height*0.5 - s*Math.sin(angle) -thumb.height/2

		if (!hasTouch)
			return

		if (distance > 1.3) {
			_translate.dstX = Math.min(
						root.x + _translate.x + (distance-1.1) * _circleRadius * Math.cos(angle),
						(maxWidth > 0 ? maxWidth : root.parent.width) - root.width/2
						)
			_translate.dstY = Math.max(
						root.parent.height - maxHeight - root.height/2,
						_translate.dstY = root.y + _translate.y - (distance-1.1) * _circleRadius * Math.sin(angle)
						)
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

	function reset() {
		_behaviorX.enabled = false
		_behaviorY.enabled = false
		_translate.dstX = x - _innerHPadding
		_translate.dstY = y + _innerVPadding
		_timer.triggered()
		_behaviorX.enabled = true
		_behaviorY.enabled = true
	}

	Component.onCompleted: reset()
}
