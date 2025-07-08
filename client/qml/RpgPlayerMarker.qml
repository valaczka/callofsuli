import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property RpgPlayer player: target

	visible: player && player.hp > 0 && (!player.isHiding || player.game.controlledPlayer === player)
}



