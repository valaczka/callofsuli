import Bacon2D 1.0
import QtQuick 2.15
import COS.Client 1.0
import "Style"

Rectangle {
	id: root
	visible: enemyPrivate && enemyPrivate.player && enemyPrivate.player.enemy != enemyPrivate
	color: enemyPrivate.attackRunning ? CosStyle.colorEnemyMarkerAttack : CosStyle.colorEnemyMarker
	height: 1
	y: parent.height*0.2

	property GameEnemyPrivate enemyPrivate: null
	property Item playerItem: null

	onPlayerItemChanged: playerRectSet()

	Connections {
		target: enemyPrivate.player && playerItem ? playerItem : null
		function onXChanged(x) {
			playerRectSet()
		}

	}

	Connections {
		target: enemyPrivate
		function onPlayerChanged(player) {
			if (player)
				playerRectSet()
			else {
				root.destroy()
			}
		}
	}

	Timer {
		id: timerBlink
		running: !enemyPrivate.attackRunning && enemyPrivate.msecLeftAttack <= 2000
		interval: 150
		triggeredOnStart: true
		repeat: true
		onTriggered: {
			root.color = (root.color == CosStyle.colorEnemyMarker ? CosStyle.colorEnemyMarkerAttack : CosStyle.colorEnemyMarker)
		}
		onRunningChanged: if (!running)
							  root.color = enemyPrivate.attackRunning ? CosStyle.colorEnemyMarkerAttack : CosStyle.colorEnemyMarker
	}


	function playerRectSet() {
		if (playerItem) {
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
		}
	}
}
