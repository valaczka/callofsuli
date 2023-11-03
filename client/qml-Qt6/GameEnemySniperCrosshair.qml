import QtQuick
import CallOfSuli


AnimatedImage {
	id: root
	visible: sniper && sniper.player
	source: "qrc:/internal/game/sniper_crosshair.gif"

	property GameEnemySniperImpl sniper: null

	anchors.centerIn: sniper && sniper.player ? sniper.player : undefined

	onSniperChanged: if (!sniper)
						 root.destroy()

	Connections {
		target: sniper
		function onPlayerChanged(player) {
			if (!player)
				root.destroy()
		}

		function onIsAliveChanged() {
			if (!sniper.isAlive)
				root.destroy()
		}
	}
}
