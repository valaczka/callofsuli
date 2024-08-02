import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

TiledPlayerMarker {
	id: root

	property IsometricEnemy enemy: target

	visible: enemy && enemy.game.controlledPlayer &&
			 (enemy.player === enemy.game.controlledPlayer ||
			  enemy.game.controlledPlayer.enemy === enemy) &&
			 enemy.hp > 0

	progressBar.width: Math.min(30, root.width)
	progressBar.color: Qaterial.Colors.red500
}



