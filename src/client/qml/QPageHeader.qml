import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control
	anchors.top: parent.top
	anchors.left: parent.left
	anchors.right: parent.right
	color: CosStyle.colorPrimaryDark

	height: Math.max(leftLoader.height, rightLoader.height, row.height)+1

	property bool isSelectorMode: false
	property alias labelCountText: labelCount.text

	property alias rightLoader: rightLoader
	property alias leftLoader: leftLoader

	signal selectAll()
	signal deselectAll()

	Item {
		id: mainItem

		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left
		anchors.right: rightLoader.left

		Loader {
			id: leftLoader
			width: parent.width
			anchors.verticalCenter: parent.verticalCenter

			visible: opacity != 0

			Behavior on opacity { NumberAnimation { duration: 125 } }
		}

		RowLayout {
			id: row

			width: parent.width
			anchors.verticalCenter: parent.verticalCenter

			opacity: 0.0
			visible: opacity != 0

			Label {
				id: labelCount
				text: ""

				horizontalAlignment: Text.AlignLeft
				verticalAlignment: Text.AlignVCenter

				Layout.fillWidth: true
				Layout.fillHeight: true
			}

			ToolButton {
				id: button1
				property string buttonLabel: "M\ue5d4"

				Layout.fillWidth: false
				Layout.fillHeight: true

				Material.foreground: CosStyle.colorPrimaryLight
				Component.onCompleted: JS.setIconFont(button1, buttonLabel)
				onButtonLabelChanged: JS.setIconFont(button1, buttonLabel)

				onClicked: control.selectAll()
			}


			ToolButton {
				id: button2
				property string buttonLabel: "M\ue5d4"

				Layout.fillWidth: false
				Layout.fillHeight: true

				Material.foreground: CosStyle.colorPrimaryLight
				Component.onCompleted: JS.setIconFont(button2, buttonLabel)
				onButtonLabelChanged: JS.setIconFont(button2, buttonLabel)

				onClicked: control.deselectAll()
			}

		}

	}


	Loader {
		id: rightLoader

		anchors.verticalCenter: parent.verticalCenter
		anchors.right: parent.right
	}


	Rectangle {
		anchors.bottom: parent.bottom
		width: parent.width
		height: 1
		color: CosStyle.colorAccent
	}


	states: [
		State {
			name: "SELECTOR"
			when: isSelectorMode
			PropertyChanges {
				target: row
				opacity: 1.0
			}
			PropertyChanges {
				target: leftLoader
				opacity: 0.0
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "SELECTOR"
			SequentialAnimation {
				NumberAnimation {
					target: leftLoader
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					target: row
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
			}
		},

		Transition {
			from: "SELECTOR"
			to: "*"
			SequentialAnimation {
				NumberAnimation {
					target: row
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					target: leftLoader
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
			}
		}
	]
}

