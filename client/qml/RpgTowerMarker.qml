import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property RpgTower tower: target

	progressBar.visible: true
	progressBar.from: 0
	progressBar.to: 100
	progressBar.value: tower ? tower.load : 0

	entityHeight: 200

	progressBarColor: tower ? tower.color : Qaterial.Colors.white
	labelColor: tower ? tower.color : Qaterial.Colors.white

	visible: tower && tower.gameItem && tower.gameItem.game &&
			 tower.gameItem.game.controlledPlayer &&
			 tower.gameItem.game.controlledPlayer.tower == tower &&
			 tower.gameItem.game.controlledPlayer.hp > 0
}



