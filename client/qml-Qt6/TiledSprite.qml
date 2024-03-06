import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects

AnimatedSprite {
	id: root

	property int spriteId: 0
	property int baseLoops: 0
	property bool waitForEnd: false

	anchors.fill: parent
	loops: waitForEnd ? 1 : baseLoops > 0 ? baseLoops : AnimatedSprite.Infinite
	finishBehavior: loops != AnimatedSprite.Infinite ? AnimatedSprite.FinishAtFinalFrame : AnimatedSprite.FinishAtInitialFrame

	visible: false

	onVisibleChanged: {
		if (visible)
			start()
		else
			stop()
	}
}
