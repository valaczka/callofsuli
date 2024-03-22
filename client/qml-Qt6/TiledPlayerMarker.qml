import QtQuick
import QtQuick.Controls
import CallOfSuli


AnimatedImage {
	id: root

	required property IsometricPlayer target

	parent: target.scene

	visible: target && target.game

	y: target ? target.y-15 : 0
	x: target ? target.x+(target.width-width)/2 : 0

	source: {
		if (!target || !target.game)
			return ""

		let p = target.hp/target.maxHp

		if (target.game.followedItem != target || target.hp <= 0)
			return "qrc:/internal/game/markerClear.gif"

		if (p > 0.5)
			return "qrc:/internal/game/markerGreen.gif"
		else if (p > 0.3)
			return "qrc:/internal/game/markerYellow.gif"
		else
			return "qrc:/internal/game/markerRed.gif"
	}

	z: 99

}

