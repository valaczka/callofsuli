import QtQuick 2.15
import QtQuick.Controls 2.15

GameQuestionButton {
	id: root

	property GameQuestionDNDflow dndFlow: null
	property Item questionItem: null

	property double maximumWidth: dndFlow ? dndFlow.flow.width : 300
	property double maximumHeight: 0

	property int dragIndex: -1

	readonly property GameQuestionDNDdrop dndDrop: (parent instanceof GameQuestionDNDdrop ? parent : null)

	property list<GameQuestionDNDdrop> drops

	width: _requiredWidth > 0 ? _requiredWidth : maximumWidth > 0 ? Math.min(implicitWidth, maximumWidth) : implicitWidth
	height: _requiredHeight > 0 ? _requiredHeight : maximumHeight > 0 ? Math.min(implicitHeight, maximumHeight) : implicitHeight


	property double _requiredWidth: 0
	property double _requiredHeight: 0

	buttonType: dndDrop ? GameQuestionButton.Selected : GameQuestionButton.Neutral

	Drag.hotSpot.x: width/2
	Drag.hotSpot.y: height/2
	Drag.dragType: Drag.None

	Drag.onActiveChanged: {
		if (!dndFlow)
			return

		dndFlow.highlighted = (_handler.active && !Drag.target)
	}

	Drag.onTargetChanged: {
		if (!dndFlow)
			return

		dndFlow.highlighted = (_handler.active && !Drag.target)
	}

	onDndDropChanged: {
		if (!dndDrop)
			return

		if (!dndDrop.allowResizeToContent) {
			_requiredWidth = dndDrop.width
			_requiredHeight = dndDrop.height
		} else {
			_requiredWidth = 0
			_requiredHeight = 0
		}
	}

	onClicked: {
		if (dndDrop) {
			dropBack()
			return
		}

		if (root.drops.length) {
			for (var i=0; i<root.drops.length; ++i) {
				var d=root.drops[i]

				if (!d.currentDrag) {
					d.dropIn(root)
					return
				}
			}
		}
	}


	DragHandler {
		id: _handler
		enabled: dndFlow && questionItem
		dragThreshold: root.height/2
		onActiveChanged: {
			if (active) {
				var p = root.mapToItem(questionItem, root.x, root.y)
				root.parent = questionItem
				root.x = p.x
				root.y = p.y
				root.Drag.start()
			} else {
				var t = root.Drag.target
				var p2 = questionItem.mapToItem(dndFlow.flow, root.x, root.y)
				if (!root.Drag.target) {
					root.Drag.cancel()
					root.parent = dndFlow.flow
					root.x = p2.x
					root.y = p2.y
					root._requiredHeight = 0
					root._requiredWidth = 0
				} else {
					root.Drag.drop()
				}
			}
		}
	}

	Behavior on width {
		NumberAnimation {
			duration: 400
			easing.type: Easing.OutQuart
		}
	}

	Behavior on height {
		NumberAnimation {
			duration: 400
			easing.type: Easing.OutQuart
		}
	}

	function dropBack() {
		if (!dndFlow || !dndFlow.flow || !questionItem)
			return false

		var p2 = questionItem.mapToItem(dndFlow.flow, root.x, root.y)

		root.parent = dndFlow.flow
		root.x = p2.x
		root.y = p2.y
		root._requiredHeight = 0
		root._requiredWidth = 0

		return true
	}
}
