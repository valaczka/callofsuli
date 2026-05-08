import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property RpgEnemy enemy: target

	visible: enemy && enemy.hp > 0

	progressBar.visible: enemy && enemy.game && enemy.game.controlledPlayer &&
						 (enemy.player === enemy.game.controlledPlayer ||
						  enemy.game.controlledPlayer.enemy === enemy) &&
						 enemy.hp > 0

	progressBar.width: Math.min(30, root.width)
	progressBar.color: Qaterial.Colors.red500
}



