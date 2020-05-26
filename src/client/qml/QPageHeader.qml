import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
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

	height: Math.max(mainItem.height, rightLoader.height, col.visible ? col.height : 0)+1

	property bool isSelectorMode: false
	property alias labelCountText: labelCount.text
	property alias searchText: searchText

	property alias rightLoader: rightLoader
	property alias selectorLoader: selectorLoader
	default property alias mainItemData: mainItem.data

	signal selectAll()

	Item {
		id: mainItem

		anchors.verticalCenter: parent.verticalCenter
		anchors.left: parent.left
		anchors.right: rightLoader.status == Loader.Ready && rightLoader.item.visible ? rightLoader.left : parent.right

		height: childrenRect.height

		visible: opacity != 0

		Behavior on opacity { NumberAnimation { duration: 125 } }
	}

	Column {
		id: col

		width: mainItem.width
		anchors.verticalCenter: parent.verticalCenter

		opacity: 0.0
		visible: opacity != 0

		RowLayout {
			id: row

			width: parent.width

			QLabel {
				id: labelCount
				text: ""

				horizontalAlignment: Text.AlignLeft
				verticalAlignment: Text.AlignVCenter

				Layout.fillWidth: false
				Layout.fillHeight: true
			}

			QTextField {
				id: searchText
				Layout.fillWidth: true

				placeholderText: qsTr("Keresés...")
			}

			QToolButton {
				id: button1
				icon.source: CosStyle.iconSelectAll

				Layout.fillWidth: false
				Layout.fillHeight: true

				onClicked: control.selectAll()
			}
		}

		Loader {
			id: selectorLoader

			width: parent.width
		}
	}


	Loader {
		id: rightLoader

		anchors.top: parent.top
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
				target: col
				opacity: 1.0
			}
			PropertyChanges {
				target: mainItem
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
					target: mainItem
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					target: col
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
					target: col
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
				NumberAnimation {
					target: mainItem
					property: "opacity"
					duration: 75
					easing.type: Easing.InOutQuad
				}
			}
		}
	]
}

