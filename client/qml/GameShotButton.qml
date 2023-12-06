import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


GameButton {
	id: shotButton
	size: 55

	property GamePlayerImpl player: null

	width: Math.min(100, control.width*0.5)
	height: Math.min(100, control.width*0.4)

	visible: player && player.isAlive

	readonly property bool enemyAimed: player && player.enemy

	color: enemyAimed ? Client.Utils.colorSetAlpha(Qaterial.Colors.red700, 0.7) : "transparent"

	border.color: enemyAimed ? "black" : "white"

	fontImage.icon: "qrc:/internal/game/target1.svg"
	fontImage.color: "white"
	fontImage.opacity: enemyAimed ? 0.6 : 1.0
	//tap.enabled: !game.question
	tap.onTapped: if (player) player.shot()

	Connections {
		target: player

		function onAttack() {
			tapAnim.start()
		}
	}
}
