import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.3
import "Style"
import "JScript.js" as JS


Button {
	id: control

	property var themeColors: CosStyle.buttonThemeDefault

	property color textColor: themeColors[0]
	property color backgroundColor: themeColors[1]
	property color borderColor: themeColors[2]
	property color blinkColor: themeColors[3]

	property bool animationEnabled: true
	readonly property bool _blinkActive: activeFocus && enabled && animationEnabled

	Material.foreground: textColor

	property color baseColor: backgroundColor

	Material.background: baseColor

	font.capitalization: Font.AllUppercase


	font.pixelSize: CosStyle.pixelSize

	/*background: Rectangle {
		id: bgRect
		implicitWidth: 10
		implicitHeight: 10


		color: control.hovered ? Qt.lighter(baseColor, 1.3) : baseColor
		opacity: enabled ? 1 : 0.3
		border.color: control.borderColor
		border.width: control.hovered
	}*/

	on_BlinkActiveChanged: {
		if (_blinkActive)
			blinkAnimation.start()
		else
			blinkAnimation.stop()
	}

	onThemeColorsChanged: {
		blinkAnimation.alwaysRunToEnd = false
		blinkAnimation.stop()
		animDestColor.to = themeColors[1]
		animBlinkColor.to = themeColors[3]
		blinkAnimation.alwaysRunToEnd = true
		if (_blinkActive)
			blinkAnimation.start()
	}


	SequentialAnimation {
		id: blinkAnimation
		loops: Animation.Infinite
		alwaysRunToEnd: true

		ColorAnimation {
			id: animBlinkColor
			target: control
			property: "baseColor"
			to: control.blinkColor
			duration: 125
		}

		PauseAnimation {
			duration: 125
		}

		ColorAnimation {
			id: animDestColor
			target: control
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