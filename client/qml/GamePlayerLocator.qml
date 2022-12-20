import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: control

	width: 200
	height: 200

	z: 10

	property color color: Qaterial.Colors.amber200
	property int count: 3
	property int _created: 0

	Component {
		id: rectangleComponent
		Rectangle {
			id: rect1
			width: control.width
			height: control.height
			radius: control.width/2
			color: control.color

			property bool last: false

			SequentialAnimation {
				running: true
				ParallelAnimation {
					PropertyAnimation {
						target: rect1
						property: "scale"
						from: 0.0
						to: 1.0
						easing.type: Easing.OutQuint
						duration: 1250
					}
					PropertyAnimation {
						target: rect1
						property: "opacity"
						from: 0.0
						to: 0.2
						duration: 550
					}
				}
				PropertyAnimation {
					target: rect1
					property: "opacity"
					to: 0.0
					easing.type: Easing.InQuad
					duration: 750
				}
				ScriptAction {
					script: {
						rect1.destroy()
						if (rect1.last) {
							control.destroy()
						}
					}
				}
			}
		}
	}


	Timer {
		id: timer

		running: _created <= count
		interval: 350
		triggeredOnStart: true

		onTriggered: {
			_created++

			rectangleComponent.createObject(control, {last: _created >= count})
		}
	}

}
