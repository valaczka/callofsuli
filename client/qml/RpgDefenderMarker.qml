import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property RpgDefender defender: target

	visible: defender && defender.hp > 0
}



