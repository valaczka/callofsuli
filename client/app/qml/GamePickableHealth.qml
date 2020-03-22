import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."

GamePickable {
	id: item

	targetObject: GamePickablePrivate {
		type: GamePickable.PickableHealth
		game: cosGame
		data: pickableData
	}

	contentItem: AnimatedImage {
		source: "qrc:/internal/game/powerup.gif"
		width: 30
		height: 30
		speed: 0.75
		fillMode: Image.PreserveAspectFit
	}
}
