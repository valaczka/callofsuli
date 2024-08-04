import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property IsometricEnemy enemy: target

	readonly property bool _isWerebear: enemy && enemy.enemyType == 1

	visible: enemy && enemy.game.controlledPlayer &&
			 (enemy.player === enemy.game.controlledPlayer ||
			  enemy.game.controlledPlayer.enemy === enemy) &&
			 enemy.hp > 0

	yDistance: _isWerebear ? -20 : 20

	progressBar.width: Math.min(_isWerebear ? 60 : 30, root.width)
	progressBar.color: Qaterial.Colors.red500
}



