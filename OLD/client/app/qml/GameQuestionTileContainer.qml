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
	}

	Flickable {
		id: flick

		anchors.fill: parent

		clip: true

		contentWidth: _flowRect.width
		contentHeight: _flowRect.height

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick

		ScrollIndicator.vertical: ScrollIndicator { }

		Item {
			id: _flowRect

			width: flick.width
			height: flow.height

			Flow {
				id: flow
				width: parent.width-control.paddingLeft-control.paddingRight
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
					obj.anchors.centerIn = undefined
					obj.fillParentWidth = false
					obj.parent = flow
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
