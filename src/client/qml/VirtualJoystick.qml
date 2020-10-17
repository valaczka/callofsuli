import QtQuick 2.12

Item {
	id: root
	width: joystick.width
	height: joystick.height

	signal joystickMoved(double x, double y);

	Rectangle {
		id: joystick

		width: 160
		height: 160
		radius: 80

		color: "white"

		border.color: "black"
		border.width: 2

		opacity: 0.3


		property real angle : 0
		property real distance : 0

		anchors.centerIn: parent

		ParallelAnimation {
			id: returnAnimation
			NumberAnimation { target: thumb.anchors; property: "horizontalCenterOffset";
				to: 0; duration: 200; easing.type: Easing.OutSine }
			NumberAnimation { target: thumb.anchors; property: "verticalCenterOffset";
				to: 0; duration: 200; easing.type: Easing.OutSine }
		}

		MouseArea {
			id: mouse
			property real fingerAngle : Math.atan2(mouseX, mouseY)
			property int mcx : mouseX - width * 0.5
			property int mcy : mouseY - height * 0.5
			property bool fingerInBounds : fingerDistance2 < distanceBound2
			property real fingerDistance2 : mcx * mcx + mcy * mcy
			property real distanceBound : width * 0.5 - thumb.width * 0.5
			property real distanceBound2 : distanceBound * distanceBound

			property double signal_x : (mouseX - joystick.width/2) / distanceBound
			property double signal_y : -(mouseY - joystick.height/2) / distanceBound

			anchors.fill: parent

			onPressed: {
				returnAnimation.stop();
			}

			onReleased: {
				returnAnimation.restart()
				joystickMoved(0, 0);
			}

			onPositionChanged: {
				if (fingerInBounds) {
					thumb.anchors.horizontalCenterOffset = mcx
					thumb.anchors.verticalCenterOffset = mcy
				} else {
					var angle = Math.atan2(mcy, mcx)
					thumb.anchors.horizontalCenterOffset = Math.cos(angle) * distanceBound
					thumb.anchors.verticalCenterOffset = Math.sin(angle) * distanceBound
				}

				// Fire the signal to indicate the joystick has moved
				angle = Math.atan2(signal_y, signal_x)

				if(fingerInBounds) {
					joystickMoved(
						Math.cos(angle) * Math.sqrt(fingerDistance2) / distanceBound,
						Math.sin(angle) * Math.sqrt(fingerDistance2) / distanceBound
					);
				} else {
					joystickMoved(
						Math.cos(angle) * 1,
						Math.sin(angle) * 1
					);
				}
			}
		}

	}

	Rectangle {
		id: thumb
		width: 52
		height: 52
		radius: 26
		color: "white"
		anchors.centerIn: joystick

		border.color: "black"
		border.width: 2

		opacity: 0.6
	}
}
