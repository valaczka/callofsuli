import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."

GamePickable {
	id: control

	property alias type: itemPrivate.type
	property url image: ""

	property real imageWidth: 25
	property real imageHeight: 25
	property real imageSourceWidth: -1
	property real imageSourceHeight: -1

	targetObject: GamePickablePrivate {
		id: itemPrivate
		game: cosGame
		data: pickableData
	}

	contentItem: Item {
		id: root

		width: lbl.width
		height: lbl.height

		Image {
			id: lbl
			source: control.image
			width: control.imageWidth
			height: control.imageHeight
			fillMode: Image.PreserveAspectFit

			sourceSize.width: control.imageSourceWidth > -1 ? control.imageSourceWidth : undefined
			sourceSize.height: control.imageSourceHeight > -1 ? control.imageSourceHeight : undefined
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
