import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."

GamePickable {
	id: control

	property alias type: itemPrivate.type
	property url image: ""
	property color color: "yellow"

	property real imageWidth: 30
	property real imageHeight: 30
	property real imageSourceSize: 30

	targetObject: GamePickablePrivate {
		id: itemPrivate
		game: cosGame
		data: pickableData
	}

	contentItem: Item {
		id: root

		width: lbl.width
		height: lbl.height

		QFontImage {
			id: lbl
			icon: control.image
			width: control.imageWidth
			height: control.imageHeight
			fillMode: Image.PreserveAspectFit

			size: control.imageSourceSize
			color: control.color
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
