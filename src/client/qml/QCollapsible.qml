import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Rectangle {
	id: control

	width: parent.width

	property int contentHeight: content.childrenRect.height+10
	property alias headerHeight: headerRect.height

	height: headerRect.height

	property bool collapsed: false
	property alias title: title.text

	default property alias contents: content.data

	color: "transparent"

	Rectangle {
		id: headerRect
		width: parent.width
		height: Math.max(title.implicitHeight, arrow.implicitHeight, CosStyle.halfLineHeight)

		color: JS.setColorAlpha(CosStyle.colorPrimaryDarkest, 0.5)

		QFontImage {
			id: arrow
			anchors.left: parent.left
			anchors.verticalCenter: parent.verticalCenter

			size: CosStyle.pixelSize*1.4
			color: CosStyle.colorPrimaryLighter

			icon: CosStyle.iconDown

			rotation: -90
			transformOrigin: Item.Center
		}

		QLabel {
			id: title
			anchors.left: arrow.right
			anchors.verticalCenter: parent.verticalCenter
			width: parent.width-arrow.width
			font.pixelSize: CosStyle.pixelSize*0.85
			font.weight: Font.DemiBold
			font.capitalization: Font.AllUppercase
			color: CosStyle.colorAccentLighter
			elide: Text.ElideRight
		}

		Rectangle {
			width: parent.width
			height: 1
			anchors.bottom: parent.bottom
			color: CosStyle.colorPrimaryDark
		}

		MouseArea {
			anchors.fill: parent
			acceptedButtons: Qt.LeftButton
			onClicked: collapsed = !collapsed
		}
	}

	Item {
		id: content

		anchors.top: headerRect.bottom
		anchors.topMargin: 5
		anchors.left: parent.left
		anchors.right: parent.right

		opacity: 0.0
		visible: opacity
	}

	states: [
		State {
			name: "VISIBLE"
			when: !control.collapsed

			PropertyChanges {
				target: arrow
				rotation: 0
			}

			PropertyChanges {
				target: control
				height: headerRect.height+contentHeight+1
			}

			PropertyChanges {
				target: content
				opacity: 1.0
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "VISIBLE"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: arrow
						property: "rotation"
						duration: 225
					}
					PropertyAnimation {
						target: control
						property: "height"
						duration: 225
						easing.type: Easing.OutQuint
					}
				}
				PropertyAnimation {
					target: content
					property: "opacity"
					duration: 175
					easing.type: Easing.InQuad
				}
			}
		},
		Transition {
			from: "VISIBLE"
			to: "*"
			SequentialAnimation {
				PropertyAnimation {
					target: content
					property: "opacity"
					duration: 125
					easing.type: Easing.OutQuad
				}
				ParallelAnimation {
					PropertyAnimation {
						target: arrow
						property: "rotation"
						duration: 175
					}
					PropertyAnimation {
						target: control
						property: "height"
						duration: 175
						easing.type: Easing.InQuint
					}
				}
			}
		}
	]

}

