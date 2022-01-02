import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "Style"
import "."

GamePickable {
	id: item

	targetObject: GamePickablePrivate {
		type: GamePickablePrivate.PickablePliers
		game: cosGame
		data: pickableData
	}

	contentItem: QFontImage {
		icon: CosStyle.iconSetup
		width: 30
		height: 30
		size: 30
		color: "brown"
	}
}
