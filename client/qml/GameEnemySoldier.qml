import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Box2D 2.0
import QtGraphicalEffects 1.0


GameEnemySoldierPrivate {
	id: control


	/*property GameEntityPrivate entityPrivate: null
	property alias spriteSequence: spriteSequence


	property alias glowColor: glow.color
	property bool glowEnabled: false
	property bool _glowForced: true

	property alias overlayColor: overlay.color
	property bool overlayEnabled: false

	onHpValueChanged: {
		if (entityPrivate && entityPrivate.maxHp>0)
			hpProgress.to = entityPrivate.maxHp
		else if (hpValue>hpProgress.to)
			hpProgress.to = hpValue
	}

	ProgressBar {
		id: hpProgress
		visible: false

		width: entityPrivate ? entityPrivate.boundBox.width+6 : 30
		x: entityPrivate ? entityPrivate.boundBox.x-3 : 0
		y: entityPrivate ? entityPrivate.boundBox.y-10 : -5
		height: 2

		from: 0
		to: (entityPrivate && entityPrivate.maxHp>0) ? entityPrivate.maxHp : 0
		value: hpValue

		Behavior on value {
			NumberAnimation { duration: 175; easing.type: Easing.InOutQuad }
		}

		background: Rectangle {
			implicitWidth: 200
			implicitHeight: 2
			color: hpColor
			radius: 0
			opacity: 0.3
		}

		contentItem: Item {
			implicitWidth: 200
			implicitHeight: 2

			Rectangle {
				width: hpProgress.visualPosition * parent.width
				height: parent.height
				radius: 0
				color: hpColor
			}
		}
	}*/


/*

	Connections {
		target: entityPrivate ? entityPrivate : null
		function onDie() {
			dieAnimation.start()
		}
	}

	SequentialAnimation {
		id: dieAnimation
		running: false
		PropertyAnimation {
			target: root
			property: "opacity"
			to: 0
			duration: 150
		}
		ScriptAction {
			script: root.destroy()
		}
	}


*/


}

