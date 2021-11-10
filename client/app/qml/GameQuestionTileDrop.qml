import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


DropArea {
	id: control
	width: implicitWidth
	height: implicitHeight

	implicitWidth: CosStyle.pixelSize*3
	implicitHeight: 20

	property GameQuestionTileDrag currentDrag: null
	property string tileKeys: "fillout"
	property bool isWrong: false

	keys: currentDrag ? ["-"] : tileKeys

	Rectangle {
		id: target
		anchors.fill: parent
		color: (control.currentDrag && !control.currentDrag.drag.active) ? "transparent"
																		 : (isWrong ? CosStyle.colorErrorDarker : CosStyle.colorAccentDarker)
		border.width: control.currentDrag ? 0 : 1
		border.color: CosStyle.colorAccentLight
		radius: 2

		states: State {
			when: control.containsDrag
			PropertyChanges {
				target: target
				color: CosStyle.colorAccentLighter
			}
		}

		function dragOut() {
			control.currentDrag = null
		}
	}

	states: [
		State {
			name: "DRAG"
			when: target.children.length
			PropertyChanges {
				target: control
				width: Math.max(target.children[0].width, control.implicitWidth)
				height: Math.max(target.children[0].height, control.implicitHeight)
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "*"
			PropertyAnimation {
				target: control
				properties: "width, height"
				duration: 125
				easing.type: Easing.OutQuad
			}
		}
	]


	function dropIn(obj) {
		obj.parent.dragOut()
		obj.parent = target
		obj.anchors.centerIn = target
		currentDrag = obj
	}

}
