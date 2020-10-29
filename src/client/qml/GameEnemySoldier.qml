import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0

GameEntity {
	id: root
	sleepingAllowed: false
	width: spriteSequence.width
	height: spriteSequence.height

	entityPrivate: ep

	GameEnemySoldierPrivate {
		id: ep

		onDie: {
			console.debug("DIE")
		}

		onMovingChanged: setSprite()
		onAtBoundChanged: setSprite()
	}

	Behavior on x {
		enabled: ep.moving
		NumberAnimation {
			duration: 50
			easing.type: Easing.InOutQuart
		}
	}



	function setSprite() {
		if (!ep.atBound && ep.moving) {
			spriteSequence.jumpTo("walk")
		}
		else {
			spriteSequence.jumpTo("idle")
		}
	}
}

