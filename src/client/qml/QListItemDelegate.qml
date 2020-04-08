import QtQuick 2.12
import QtQuick.Controls 2.12
import "."
import "Style"


QListView {
	id: view

	property bool modelTitleSet: true
	property bool modelSubtitleSet: false
	property bool modelRightSet: false
	property bool modelEnabledSet: false
	property bool modelToolTipSet: false

	property int delegateHeight: 48

	signal clicked(int index)
	signal rightClicked(int index)
	signal longPressed(int index)

	model: ListModel {	}

	delegate: Rectangle {
		id: item
		height: view.delegateHeight
		width: view.width

		property bool enabled: true

		property string labelTitle: modelTitleSet ? model.labelTitle : ""
		property string labelSubtitle: modelSubtitleSet ? model.labelSubtitle : ""
		property string labelRight: modelRightSet ? model.labelRight : ""

		color: (area.containsMouse || item.activeFocus) ?
				   CosStyle.colorPrimaryLighter :
				   CosStyle.colorPrimary

		signal clicked()
		signal rightClicked()
		signal longPressed()

		ToolTip.text: modelToolTipSet ? model.toolTip : ""
		ToolTip.visible: (modelToolTipSet ? model.toolTip.length : false) && (area.containsMouse || area.pressed)
		ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval

		Behavior on color { ColorAnimation { duration: 125 } }

		Column {
			anchors.left: parent.left
			anchors.right: lblRight.left
			anchors.verticalCenter: parent.verticalCenter

			Label {
				id: lblTitle
				text: item.labelTitle
				color: "black"
				maximumLineCount: 1
				elide: Text.ElideRight
			}

			Label {
				id: lblSubtitle
				text: item.labelSubtitle
				color: "black"
			}
		}

		Label {
			id: lblRight
			anchors.right: parent.right
			anchors.verticalCenter: parent.verticalCenter

			text: item.labelRight

			color: "black"
		}


		MouseArea {
			id: area
			anchors.fill: parent
			hoverEnabled: true
			acceptedButtons: Qt.LeftButton | Qt.RightButton

			onClicked: {
				if (mouse.button == Qt.RightButton)
					item.rightClicked()
				else
					item.clicked()

				item.forceActiveFocus()
			}

			onPressAndHold: {
				item.longPressed()
				item.forceActiveFocus()
			}
		}

		onClicked: {
			view.currentIndex = index
			view.clicked(index)
		}

		onLongPressed: {
			view.currentIndex = index
			view.longPressed(index)
		}

		onRightClicked: {
			view.currentIndex = index
			view.rightClicked(index)
		}

		states: [
			State {
				name: "Pressed"
				when: area.pressed

				PropertyChanges {
					target: item
					scale: 0.85
				}
			}
		]

		transitions: [
			Transition {
				PropertyAnimation {
					target: item
					property: "scale"
					duration: 125
					easing.type: Easing.InOutCubic
				}
			}
		]

		SequentialAnimation {
			id: mousePressAnimation
			loops: 1
			running: false

			PropertyAction {
				target: item
				property: "state"
				value: "Pressed"
			}

			PauseAnimation {
				duration: 75
			}

			PropertyAction {
				target: item
				property: "state"
				value: ""
			}
		}

		Keys.onEnterPressed: {
			mousePressAnimation.start()
			item.clicked()
		}
		Keys.onReturnPressed: {
			mousePressAnimation.start()
			item.clicked()
		}

		Keys.onSpacePressed: {
			mousePressAnimation.start()
			item.clicked()
		}

		function press() {
			mousePressAnimation.start()
			item.clicked()
		}
	}
}
