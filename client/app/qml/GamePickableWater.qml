import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "Style"
import "."

GamePickable {
	id: item

	targetObject: GamePickablePrivate {
		type: GamePickablePrivate.PickableWater
		game: cosGame
		data: pickableData
	}

	contentItem: QFontImage {
		id: root
		icon: CosStyle.iconDrawer
		width: 25
		height: 25
		color: "blue"

		SequentialAnimation {
			running: true
			loops: Animation.Infinite
			ParallelAnimation {
				PropertyAnimation {
					target: root
					property: "scale"
					to: 1.2
					duration: 500
				}
			}
			ParallelAnimation {
				PropertyAnimation {
					target: root
					property: "scale"
					to: 1.0
					duration: 500
				}
			}
		}
	}
}
