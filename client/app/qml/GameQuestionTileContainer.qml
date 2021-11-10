import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


DropArea {
	id: control

	implicitHeight: Math.max(flow.height, 150)
	implicitWidth: 300

	property real paddingLeft: 10
	property real paddingRight: 10

	keys: [ "fillout" ]

	property alias flow: flow

	Rectangle {
		id: rect
		color: "transparent"
		anchors.fill: parent

		states: State {
			when: control.containsDrag
			PropertyChanges {
				target: rect
				color: CosStyle.colorPrimaryDark
			}
		}

		Flickable {
			id: flick

			width: parent.width
			height: Math.min(parent.height-10, flick.contentHeight)
			anchors.verticalCenter: parent.verticalCenter

			clip: true

			contentWidth: flow.width
			contentHeight: flow.height

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick


			Flow {
				id: flow
				width: flick.width-control.paddingLeft-control.paddingRight
				x: control.paddingLeft

				spacing: 10

				move: Transition {
					NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutQuad }
				}

				Behavior on height {
					SmoothedAnimation { duration: 125 }
				}

				Behavior on width {
					SmoothedAnimation { duration: 125 }
				}

				function dropIn(obj) {
					obj.parent.dragOut()
					obj.parent = flow
					obj.anchors.centerIn = undefined
				}

				function dragOut() {

				}
			}
		}
	}

	function dropIn(obj) {
		flow.dropIn(obj)
	}
}
