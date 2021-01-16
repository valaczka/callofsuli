import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"
import "."

GamePickable {
	id: item

	targetObject: GamePickablePrivate {
		type: GameEnemyData.PickableTime
		game: cosGame
		data: pickableData
	}

	contentItem: Item {
		id: root

		width: lbl.width
		height: lbl.height
		QLabel {
			id: lbl
			text: pickableData ? pickableData.text : ""
			color: CosStyle.colorOKLighter
			font.weight: Font.Bold
			font.pixelSize: 14
			visible: false
		}
		Glow {
			anchors.fill: lbl
			source: lbl
			color: "black"
			radius: 2
			spread: 0.5
			samples: 5
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
				ColorAnimation {
					target: lbl
					property: "color"
					to: CosStyle.colorOKLight
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
				ColorAnimation {
					target: lbl
					property: "color"
					to: CosStyle.colorOKLighter
					duration: 500
				}
			}
		}
	}
}
