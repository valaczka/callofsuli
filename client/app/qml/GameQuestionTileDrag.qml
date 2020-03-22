import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	width: tile.width
	height: tile.height

	implicitWidth: tile.implicitWidth
	implicitHeight: tile.implicitHeight

	property Flow dropFlow: null
	property Item mainContainer: null

	property var tileData: null
	property string tileKeys: "fillout"

	property alias text: tile.text
	property alias type: tile.type
	property alias interactive: tile.interactive
	property bool fillParentWidth: false

	readonly property bool dragActive: tile.Drag.active

	signal parentAnimationFinished()

	enabled: tile.interactive

	Connections {
		target: parent

		function onWidthChanged() {
			if (parent.width > 0 && !parent.autoResize) {
				tile.width = fillParentWidth ? parent.width : Math.min(tile.implicitWidth, parent.width)
			}
		}
	}

	GameQuestionButton {
		id: tile

		width: implicitWidth
		height: implicitHeight

		isDrag: true
		Drag.keys: tileKeys

		onDragReleased: {
			if (tile.Drag.target) {
				tile.Drag.target.dropIn(control)
			} else if (dropFlow) {
				dropFlow.dropIn(control)
			}
		}

		states: State {
			name: "DragActive"
			when: tile.Drag.active

			ParentChange {
				target: tile
				parent: mainContainer
			}
		}

		transitions: [
			Transition {
				from: "DragActive"
				to: ""

				SequentialAnimation {
					ParentAnimation {

					}

					ScriptAction {
						script: {
							if (control.parent.width > 0 && !control.parent.autoResize) {
								tile.width = fillParentWidth ? control.parent.width : Math.min(tile.implicitWidth, control.parent.width)
							} else {
								tile.width = tile.implicitWidth
							}
							parentAnimationFinished()
						}
					}
				}

			}
		]
	}

}
