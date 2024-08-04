import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

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



