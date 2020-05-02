import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS


Button {
	id: control

	property color textColor: CosStyle.colorPrimaryLight
	property color backgroundColor: CosStyle.colorPrimaryDarker
	property color borderColor: CosStyle.colorPrimaryLighter
	property color blinkColor: CosStyle.colorAccentDarker

	property bool animationEnabled: true

	Material.foreground: textColor

	background: Rectangle {
		id: bgRect
		implicitWidth: 10
		implicitHeight: 10

		property color baseColor: control.backgroundColor

		color: control.hovered ? Qt.lighter(baseColor, 1.3) : baseColor
		opacity: enabled ? 1 : 0.3
		border.color: control.borderColor
		border.width: control.hovered
	}



	SequentialAnimation {
		id: blinkAnimation
		loops: Animation.Infinite
		running: control.activeFocus && control.animationEnabled && control.enabled
		alwaysRunToEnd: true

		ColorAnimation {
			target: bgRect
			property: "baseColor"
			to: control.blinkColor
			duration: 125
		}

		PauseAnimation {
			duration: 125
		}

		ColorAnimation {
			id: animDestColor
			target: bgRect
			property: "baseColor"
			to: control.backgroundColor
			duration: 75
		}
	}

	states: [
		State {
			name: "Pressed"
			when: control.pressed

			PropertyChanges {
				target: control
				scale: 0.85
			}
		}
	]

	transitions: [
		Transition {
			PropertyAnimation {
				target: control
				property: "scale"
				duration: 125
				easing.type: Easing.InOutCubic
			}
		}
	]

	SequentialAnimation {
		id: mousePressAnimation
		loops: 1
		running: false

		PropertyAction {
			target: control
			property: "state"
			value: "Pressed"
		}

		PauseAnimation {
			duration: 75
		}

		PropertyAction {
			target: control
			property: "state"
			value: ""
		}
	}

	Keys.onEnterPressed: press()

	Keys.onReturnPressed: press()

	function press() {
		if (control.enabled) {
			mousePressAnimation.start()
			control.clicked()
		}
	}
}
