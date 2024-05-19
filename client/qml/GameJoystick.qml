import QtQuick 2.15

Item {
	id: root
	width: size
	height: size

	property real size: 160

	property real currentX: 0.0
	property real currentY: 0.0
	property real currentAngle: 0.0
	property real currentDistance: 0.0			// x*x + y*y (!!!)
	property bool hasTouch: false

	signal joystickMoved(real x, real y)
	signal directionChanged(real angle, real distance)


	onWidthChanged: moveThumb(root.width/2, root.height/2)
	onHeightChanged: moveThumb(root.width/2, root.height/2)

	Rectangle {
		width: 1
		x: root.width/2
		height: ((root.height-thumb.height)/2)-5
		color: "white"
		opacity: 0.7
		visible: !hasTouch && root.enabled
	}

	Rectangle {
		width: 1
		x: root.width/2
		y: ((root.height+thumb.height)/2)+5
		height: ((root.height-thumb.height)/2)-5
		color: "white"
		opacity: 0.7
		visible: !hasTouch && root.enabled
	}


	Rectangle {
		height: 1
		y: root.height/2
		width: ((root.width-thumb.width)/2)-5
		color: "white"
		opacity: 0.7
		visible: !hasTouch && root.enabled
	}

	Rectangle {
		height: 1
		y: root.height/2
		x: ((root.width+thumb.width)/2)+5
		width: ((root.width-thumb.width)/2)-5
		color: "white"
		opacity: 0.7
		visible: !hasTouch && root.enabled
	}

	Rectangle {
		id: thumb
		width: 40
		height: 40
		radius: 20
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

		onXChanged: calculate()
		onYChanged: calculate()

		function calculate() {
			if (!hasTouch)
				return

			var dx = (2*x+width-root.width)/(root.width-width)
			var dy = -(2*y+height-root.height)/(root.height-height)

			currentX = dx
			currentY = dy
			currentAngle = Math.atan2(dy, dx)
			currentDistance = dx*dx + dy*dy//Math.abs(-dx)+Math.abs(-dy)

			joystickMoved(currentX, currentY)
			directionChanged(currentAngle, currentDistance)
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
		thumb.x = Math.min(Math.max(0, centerX-thumb.width/2), root.width-thumb.width)
		thumb.y = Math.min(Math.max(0, centerY-thumb.height/2), root.height-thumb.height)
	}

	function moveThumbRelative(dx, dy) {
		thumb.x = Math.min(Math.max(0, (root.width*dx)-thumb.width/2), root.width-thumb.width)
		thumb.y = Math.min(Math.max(0, (root.height*dy)-thumb.height/2), root.height-thumb.height)
	}
}
