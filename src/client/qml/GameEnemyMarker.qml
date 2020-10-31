import Bacon2D 1.0
import QtQuick 2.14
import COS.Client 1.0
import "Style"

Rectangle {
	id: root
	visible: false
	color: CosStyle.colorWarningLighter
	height: 1.5
	y: parent.height*0.2

	property GameEnemyPrivate enemyPrivate: null

	Connections {
		target: enemyPrivate && enemyPrivate.player ? enemyPrivate.player : null
		onXChanged: {
			playerRectSet()
		}
	}

	Connections {
		target: enemyPrivate ? enemyPrivate : null
		onPlayerChanged: {
			playerRectSet()
		}

		onMsecLeftAttackChanged: {
			var m = enemyPrivate.msecLeftAttack
			if (m>0)
				root.visible = true
			else
				root.visible = false
		}
	}

	Timer {
		id: timerBlink
		running: enemyPrivate && enemyPrivate.msecLeftAttack <= 2000
		interval: 150
		triggeredOnStart: true
		repeat: true
		onTriggered: {
			root.color = root.color == CosStyle.colorWarningLighter ? CosStyle.colorError : CosStyle.colorWarningLighter
		}
		onRunningChanged: if (!running)
							  root.color = CosStyle.colorWarningLighter
	}

	function playerRectSet() {
		if (enemyPrivate.player) {
			var playerItem = enemyPrivate.player.parentEntity
			var rx

			if (parent.facingLeft) {
				rx = playerItem.x+playerItem.width
				root.width = parent.x-rx
				root.x = rx-parent.x
			} else {
				rx = parent.x+parent.width
				root.width = playerItem.x-rx
				root.x = parent.width
			}
		} else {
			root.visible = false
		}
	}
}
