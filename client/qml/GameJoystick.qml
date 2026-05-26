import QtQuick
import Qaterial as Qaterial

Item {
	id: root

	property bool extendedSize: false

	width: Math.min(extendedSize ? size*3 : size*1.3,
					bounding.width > 0 ? bounding.width : parent.width)

	height: Math.min(extendedSize ? size*3 : size*1.3,
					 bounding.height > 0 ? bounding.height : parent.height)

	property real size: 120 * Qaterial.Style.pixelSizeRatio
	property real thumbSize: 40 * Qaterial.Style.pixelSizeRatio

	property real currentX: 0.0
	property real currentY: 0.0
	property real currentAngle: 0.0
	property real currentDistance: 0.0
	property bool hasTouch: false

	property rect bounding: Qt.rect(0, 0, parent.width, parent.height)
	property bool moveToTap: false

	property alias fontImage: fontImage
	property real fontImageScale: 0.75

	property alias circleVisible: _circle.visible
	property alias thumb: thumb

	readonly property real _circleRadius: thumbSize*1.5/2
	readonly property real _innerHPadding: (width-size)/2
	readonly property real _innerVPadding: (height-size)/2

	signal joystickMoved(real x, real y)
	signal directionChanged(real angle, real distance)
	signal clicked()
	signal released(bool click)

	onWidthChanged: moveThumb(root.width/2, root.height/2)
	onHeightChanged: moveThumb(root.width/2, root.height/2)
	onXChanged: reset()
	onYChanged: reset()




	transform: Translate {
		id: _translate

		property real dstX: root.x
		property real dstY: root.y

		Behavior on x {
			id: _behaviorX
			SmoothedAnimation { duration: 750; easing.type: Easing.OutCubic }
		}

		Behavior on y {
			id: _behaviorY
			SmoothedAnimation { duration: 750; easing.type: Easing.OutCubic }
		}

		x: Math.max(bounding.left-_innerHPadding, Math.min(dstX, bounding.right+_innerHPadding-root.width))-root.x
		y: Math.max(bounding.top-_innerVPadding, Math.min(dstY, bounding.bottom+_innerVPadding-root.height))-root.y
	}



	Rectangle {
		id: _circle

		width: _circleRadius*2
		height: _circleRadius*2
		anchors.centerIn: parent
		radius: _circleRadius

		color: "transparent"
		border.color: thumb.color
		border.width: 2
		opacity: root.enabled ? 0.4	 : 0.3
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

		opacity: root.enabled ? 1.0 : 0.3

		Behavior on x {
			enabled: !hasTouch
			NumberAnimation { duration: 200; easing.type: Easing.OutSine }
		}

		Behavior on y {
			enabled: !hasTouch
			NumberAnimation { duration: 200; easing.type: Easing.OutSine }
		}

		Behavior on color {
			ColorAnimation { duration: 125 }
		}

		Qaterial.Icon {
			id: fontImage
			anchors.centerIn: parent
			color: Client.Utils.colorSetAlpha("black", 0)
			size: thumbSize*fontImageScale
			width: thumbSize*fontImageScale
			height: thumbSize*fontImageScale

			Behavior on color { ColorAnimation { duration: 125 } }
		}
	}



	MultiPointTouchArea {
		anchors.fill: parent
		maximumTouchPoints: 1

		touchPoints: [
			TouchPoint {
				id: point

				property var _start: 0

				onPressedChanged: {
					if (pressed) {
						_start = new Date().getTime()
					} else {
						let diff = new Date().getTime() - _start
						let delta = Math.max(Math.abs(startX-x), Math.abs(startY-y))

						_start = 0

						let click = false

						if (delta < 20 && diff < 250) {
							click = true
							root.clicked()
						}

						root.released(click)
					}
				}
			}
		]


		onTouchUpdated: touchPoints => {
							if (touchPoints.length) {
								if (hasTouch) {
									moveThumb(touchPoints[0].x, touchPoints[0].y)
								} else if (moveToTap) {
									_behaviorX.enabled = false
									_behaviorY.enabled = false

									_translate.dstX = root.x + _translate.x + touchPoints[0].x - root.width/2
									_translate.dstY = root.y + _translate.y + touchPoints[0].y - root.height/2

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

		if (distance > 1.5) {
			_translate.dstX = root.x + _translate.x + (dx - Math.max(-1, Math.min(1, dx)))*size*0.5
			_translate.dstY = root.y + _translate.y + (dy - Math.max(-1, Math.min(1, dy)))*size*0.5
		}

		currentX = dx
		currentY = dy
		currentAngle = Math.atan2(dy, dx)
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
		_translate.dstX = x
		_translate.dstY = y
		moveThumb(width/2, height/2)
		_behaviorX.enabled = true
		_behaviorY.enabled = true
	}

	Component.onCompleted: reset()
}
