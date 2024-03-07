import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

AnimatedSprite {
	id: root

	property int baseLoops: 0
	property bool waitForEnd: false

	anchors.fill: parent
	loops: waitForEnd ? 1 : baseLoops > 0 ? baseLoops : AnimatedSprite.Infinite
	finishBehavior: AnimatedSprite.FinishAtFinalFrame

	visible: false

	onVisibleChanged: {
		if (visible)
			start()
		else
			stop()
	}
}
