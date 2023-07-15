import QtQuick 2.15

Item {
	id: root
	width: 160
	height: 160

	property bool hasTouch: false

	signal joystickMoved(real x, real y)


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

		onXChanged: {
			var dx = (2*x+width-root.width)/(root.width-width)
			var dy = -(2*y+height-root.height)/(root.height-height)
			joystickMoved(dx, dy)
		}

		onYChanged: {
			var dx = (2*x+width-root.width)/(root.width-width)
			var dy = -(2*y+height-root.height)/(root.height-height)
			joystickMoved(dx, dy)
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

		onTouchUpdated: {
			if (touchPoints.length) {
				hasTouch = true
				moveThumb(touchPoints[0].x, touchPoints[0].y)
			} else {
				hasTouch = false
				moveThumb(root.width/2, root.height/2)
			}

		}
	}




	function moveThumb(centerX, centerY) {
		thumb.x = Math.min(Math.max(0, centerX-thumb.width/2), root.width-thumb.width)
		thumb.y = Math.min(Math.max(0, centerY-thumb.height/2), root.height-thumb.height)
	}
}
