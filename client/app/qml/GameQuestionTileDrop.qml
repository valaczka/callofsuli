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

	implicitWidth: CosStyle.pixelSize*5
	implicitHeight: 20

	property GameQuestionTileDrag currentDrag: null
	property string tileKeys: "fillout"
	property bool isWrong: false
	property alias autoResize: target.autoResize

	keys: currentDrag ? ["-"] : tileKeys

	Rectangle {
		id: target
		anchors.fill: parent
		color: (control.currentDrag && !control.currentDrag.dragActive) ? "transparent"
																		: (isWrong ? CosStyle.colorErrorDarker : CosStyle.colorAccentDarker)
		border.width: control.currentDrag ? 0 : 1
		border.color: CosStyle.colorAccentLight
		radius: 2

		property bool autoResize: true

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

	Behavior on width {
		NumberAnimation {
			duration: 125
			easing.type: Easing.OutQuad
		}
	}

	Behavior on height {
		NumberAnimation {
			duration: 125
			easing.type: Easing.OutQuad
		}
	}


	Connections {
		target: currentDrag

		function onParentAnimationFinished() {
			if (autoResize)
				control.width = currentDrag.width
		}

		function onWidthChanged() {
			if (autoResize)
				control.width = currentDrag.width
		}

		function onHeightChanged() {
			control.height = currentDrag.height
		}
	}

	onCurrentDragChanged: if (!currentDrag) {
							  height = implicitHeight
							  if (autoResize)
								  width = implicitWidth
						  }

	function dropIn(obj) {
		obj.parent.dragOut()
		obj.fillParentWidth = !autoResize
		obj.parent = target
		obj.anchors.centerIn = target

		currentDrag = obj
	}


}
