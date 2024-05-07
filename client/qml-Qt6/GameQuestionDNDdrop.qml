import QtQuick
import QtQuick.Controls
import QtQml
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


DropArea {
	id: root

	implicitWidth: Qaterial.Style.gameButtonImplicitHeight*2.5
	implicitHeight: Qaterial.Style.gameButtonImplicitHeight

	property bool allowResizeToContent: true
	property bool allowReplaceContent: true
	property bool showAsError: false

	property alias placeholderText: _hint.text


	readonly property Item currentDrag: children.length > 1 ? children[1] : null

	Rectangle {
		id: target
		anchors.fill: parent
		visible: !currentDrag

		color: Qaterial.Style.backgroundColor
		border.color: Qaterial.Style.dividersColor()
		border.width: Qaterial.Style.rawButton.outlinedWidth
		radius: Qaterial.Style.rawButton.cornerRadius

		Qaterial.LabelHint1 {
			id: _hint
			width: Math.min(implicitWidth, parent.width)
			height: Math.min(implicitHeight, parent.height)
			padding: 5
			wrapMode: Text.Wrap
			horizontalAlignment: Text.AlignHCenter
			anchors.centerIn: parent
			visible: !currentDrag
			elide: Text.ElideRight
		}

		states: [
			State {
				when: root.containsDrag && root.currentDrag && root.allowReplaceContent
				PropertyChanges {
					target: target
					color: Qaterial.Style.iconColor()
					visible: true
				}

				PropertyChanges {
					target: root.currentDrag
					opacity: 0.5
				}
			},
			State {
				when: root.containsDrag
				PropertyChanges {
					target: target
					color: Qaterial.Style.iconColor()
				}
			},
			State {
				when: root.showAsError

				PropertyChanges {
					target: target
					color: Client.Utils.colorSetAlpha(Qaterial.Colors.red800, 0.4)
					border.color: Qaterial.Colors.red500
					visible: true
				}

			}

		]
	}


	function dropIn(object) {
		object.parent = root
		object.x = 0
		object.y = 0
	}

	onDropped: drop => {
		var d = drop.source

		if (!d)
			return

		if (currentDrag && !currentDrag.dropBack())
			return

		if (dropIn(d))
			drop.accept()
	}


	Binding {
		target: root
		when: currentDrag && !allowReplaceContent
		property: "keys"
		value: "---invalid---"
		restoreMode: Binding.RestoreBindingOrValue
	}


	states: [
		/*State {
			name: "containsItem"
			when: containsDrag && drag.source && drag.source.width && drag.source.height && allowResizeToContent

			PropertyChanges {
				target: root
				width: Math.max(root.implicitWidth, drag.source.width)
				height: Math.max(root.implicitHeight, drag.source.height)
			}
		},*/

		State {
			name: "hasDragResized"
			when: currentDrag && allowResizeToContent

			PropertyChanges {
				target: root
				width: currentDrag.width
				height: currentDrag.height
			}
		}
	]

	transitions: [
		Transition {
			NumberAnimation {
				properties: "width, height"
				duration: 400
				easing.type: Easing.OutQuart
			}
		}
	]


}
