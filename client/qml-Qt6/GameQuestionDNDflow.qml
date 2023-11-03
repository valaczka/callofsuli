import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property double topPadding: 10
	property double bottomPadding: 10
	property double leftPadding: 10
	property double rightPadding: 10

	property bool highlighted: false

	default property alias contentItems: _flow.children

	property alias flow: _flow

	implicitWidth: Math.max(_flow.implicitWidth, Qaterial.Style.gameButtonImplicitHeight*2)+leftPadding+rightPadding
	implicitHeight: Math.max(_flow.implicitHeight, Qaterial.Style.gameButtonImplicitHeight)+topPadding+bottomPadding

	Rectangle {
		anchors.fill: parent
		color: Qaterial.Style.iconColor()
		visible: root.highlighted
	}

	Flickable {
		id: _flick

		anchors.fill: parent

		clip: true

		boundsBehavior: Flickable.StopAtBounds
		flickableDirection: Flickable.VerticalFlick

		contentWidth: _flowItem.width
		contentHeight: _flowItem.height

		ScrollIndicator.vertical: ScrollIndicator { active: _flick.movingVertically || _flick.contentHeight > _flick.height }

		Item {
			id: _flowItem
			width: _flick.width
			height: _flow.height

			Flow {
				id: _flow

				x: root.leftPadding
				y: root.topPadding
				width: _flowItem.width-root.leftPadding-root.rightPadding

				spacing: 15

				move: Transition {
					NumberAnimation { properties: "x,y"; duration: 400; easing.type: Easing.OutQuad }
				}

				add: Transition {
					NumberAnimation { properties: "x,y"; duration: 250; easing.type: Easing.OutQuad }
				}
			}
		}
	}


	function createDND(cmp, container, prop) {
		if (!cmp) {
			console.error("Missing component")
			return null
		}

		var o = cmp.createObject(_flow, prop)

		if (!o) {
			console.error("Can't create DND object")
			return null
		}

		o.dndFlow = root
		o.questionItem = container

		return o
	}
}
