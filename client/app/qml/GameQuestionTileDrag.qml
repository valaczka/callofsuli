import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


MouseArea {
	id: control

	width: tile.implicitWidth
	height: tile.implicitHeight

	drag.target: tile

	property Flow dropFlow: null
	property Item mainContainer: null

	property var tileData: null
	property string tileKeys: "fillout"

	property alias text: tile.text
	property alias type: tile.type
	property alias interactive: tile.interactive

	enabled: tile.interactive

	onReleased: {
		if (tile.Drag.target) {
			tile.Drag.target.dropIn(control)
		} else if (dropFlow) {
			dropFlow.dropIn(control)
		}
	}

	GameQuestionButton {
		id: tile

		width: implicitWidth
		height: implicitHeight

		anchors.verticalCenter: parent.verticalCenter
		anchors.horizontalCenter: parent.horizontalCenter

		isDrag: true

		Drag.active: control.drag.active
		Drag.hotSpot.x: width/2
		Drag.hotSpot.y: height/2
		Drag.keys: tileKeys


		states: State {
			when: control.drag.active
			ParentChange {
				target: tile
				parent: mainContainer
			}

			AnchorChanges {
				target: tile
				anchors.verticalCenter: undefined
				anchors.horizontalCenter: undefined
			}
		}
	}

	transitions: Transition {
		ParentAnimation {
			NumberAnimation { properties: "x,y"; duration: 600 }
		}
	}

}
