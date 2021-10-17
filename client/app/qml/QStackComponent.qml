import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.14
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: 1200
	implicitHeight: 400

	property alias headerContent: header.children
	property alias stackView: stack
	property alias initialItem: stack.initialItem

	property real requiredWidth: 1000
	property real maximumPanelWidth: requiredWidth-200

	property QBasePage basePage: null

	Item {
		id: header
		width: parent.width
		anchors.top: parent.top
		visible: children.length
		height: childrenRect.height
	}

	StackView {
		id: stack

		property real requiredWidth: control.requiredWidth
		property real maximumPanelWidth: control.maximumPanelWidth
		readonly property bool _panelVisible: width >= requiredWidth

		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		onCurrentItemChanged: if (basePage) {
								  if (currentItem && currentItem.contextMenuFunc)
									  basePage.contextMenuFunc = currentItem.contextMenuFunc
								  else
									  basePage.contextMenuFunc = null

								  if (_panelVisible)
									  basePage.subtitle = ""
								  else
									  basePage.subtitle = currentItem.title

								  currentItem.populated()
							  }

		on_PanelVisibleChanged: if (basePage) {
			if (_panelVisible)
				basePage.subtitle = ""
			else
				basePage.subtitle = currentItem.title
		}
	}

	function pushComponent(comp, params) {
		var obj = comp.createObject(stack, params)
		if (obj === null)
			console.warning("Component create error: ", comp)
		else
			stack.push(obj)

		return obj
	}

	function layoutBack() {
		if (stack.depth > 1) {
			stack.pop()
			return true
		}

		return false
	}

}
