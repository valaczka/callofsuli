import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

TiledPlayerMarker {
	id: root

	property RpgDefender defender: target

	//entityHeight: 75

	Connections {
		target: defender

		function onHpChanged() {
			if (defender.hp <= 0)
				visible = false
		}
	}

	visible: false
}



