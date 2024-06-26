import QtQuick
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
	id: control

	visible: enemy && enemy.player
	color: enemy && enemy.enemyState == GameEnemyImpl.Attack ? Qaterial.Style.colorEnemyMarkerAttack : Qaterial.Style.colorEnemyMarkerWatch
	height: 1
	y: parent.height*0.2

	property GameEnemyImpl enemy: null

	Connections {
		target: enemy ? enemy.player : null

		function onXChanged(x) {
			playerRectSet()
		}

		function onIsAliveChanged() {
			if (!enemy.player.isAlive)
				control.destroy()
		}

	}

	onEnemyChanged: playerRectSet()

	Connections {
		target: enemy
		function onPlayerChanged(player) {
			if (player)
				playerRectSet()
			else {
				control.destroy()
			}
		}

		function onIsAliveChanged() {
			if (!enemy.isAlive)
				control.destroy()
		}
	}

	Timer {
		id: timerBlink
		running: enemy && enemy.enemyState == GameEnemyImpl.WatchPlayer && enemy.msecLeftToAttack < 1500
		interval: 150
		triggeredOnStart: true
		repeat: true
		onTriggered: {
			control.color = (control.color == Qaterial.Style.colorEnemyMarkerAttack ? Qaterial.Style.colorEnemyMarkerWatch
																					: Qaterial.Style.colorEnemyMarkerAttack)
		}
		onRunningChanged: if (!running)
							  control.color = Qaterial.Style.colorEnemyMarkerAttack
	}


	function playerRectSet() {
		if (enemy && enemy.player) {
			var rx

			if (enemy.facingLeft) {
				rx = enemy.player.x+enemy.player.width
				control.width = parent.x-rx
				control.x = rx-parent.x
			} else {
				rx = parent.x+parent.width
				control.width = enemy.player.x-rx
				control.x = parent.width
			}
		}
	}
}
