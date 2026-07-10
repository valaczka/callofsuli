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
	progressBar.to: tower && tower.lockTime > 0 ? tower.maxLockTime : 100
	progressBar.value: tower ? (tower.lockTime > 0 ? tower.lockTime : tower.load) : 0

	entityHeight: 150

	progressBarColor: tower ? tower.color : Qaterial.Colors.white
	labelColor: tower ? tower.color : Qaterial.Colors.white
}



