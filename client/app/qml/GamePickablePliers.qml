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

	contentItem: Item {
		id: root

		width: lbl.width
		height: lbl.height

		Image {
			id: lbl
			source: "qrc:/internal/game/pliers.png"
			width: 25
			height: 25
			fillMode: Image.PreserveAspectFit
		}


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
