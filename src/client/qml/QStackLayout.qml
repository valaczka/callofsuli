import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Item {
	id: control

	property bool isLayout: false
	property var components: []

	focus: false

	RowLayout {
		id: layout
		anchors.fill: parent
		visible: isLayout

		spacing: 0

		focus: false

		Repeater {
			id: layoutRepeater
			model: components.length

			Loader {
				Layout.fillHeight: true
				Layout.fillWidth: true
			}
		}
	}


	SwipeView {
		id: swipe
		anchors.fill: parent
		visible: !isLayout

		focus: false

		Repeater {
			id: swipeRepeater
			model: components.length

			Loader {
			}
		}

	}


	PageIndicator {
		id: indicator
		visible: swipe.visible && swipe.count > 1

		count: swipe.count
		currentIndex: swipe.currentIndex

		anchors.bottom: swipe.bottom
		anchors.horizontalCenter: control.horizontalCenter
	}


	function reloadComponents() {
		for (var i=0; i<components.length; ++i) {
			var c = components[i]
			if (isLayout) {
				layoutRepeater.itemAt(i).setSource(c.url, c.params)
				swipeRepeater.itemAt(i).setSource("")
			} else {
				layoutRepeater.itemAt(i).setSource("")
				swipeRepeater.itemAt(i).setSource(c.url, c.params)
			}
		}
	}

	Component.onCompleted: reloadComponents()

	/*
	function loadComponents(isLayout, list) {
		for (var i=0; i<list.length; ++i) {
			var c = list[i]
			var url = c.url
			var params = c.params

			console.debug(c, url, params)

			var comp = Qt.createComponent(url)

			if (comp.status === Component.Ready) {
				var obj = comp.createObject(control, params)
				if (obj === null) {
					console.error("Error creating object")
				} else {
					if (isLayout) {
						layout.children.push(obj)
					} else {
						swipe.addItem(obj)
					}
				}

			} else if (comp.status === Component.Error) {
				console.warn("Error loading component: ", comp.errorString())
			}
		}

		if (isLayout) {
			layout.visible = true
		} else {
			swipe.visible = true
		}
	}
	*/
}
