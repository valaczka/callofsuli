import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: control

	readonly property int itemHeight: 24
	property real maximumHeight: implicitMaximumHeight

	implicitWidth: 450
	readonly property real implicitMaximumHeight: 5*itemHeight+view.anchors.topMargin+view.anchors.bottomMargin


	height: Math.floor((maximumHeight-view.anchors.topMargin-view.anchors.bottomMargin)/itemHeight)
			*itemHeight
			+view.anchors.topMargin+view.anchors.bottomMargin

	ListView {
		id: view

		interactive: false

		anchors.fill: parent
		anchors.leftMargin: 7
		anchors.rightMargin: 10
		anchors.topMargin: 5
		anchors.bottomMargin: 5

		clip: true

		model: ListModel {
			id: messageModel
		}

		delegate: Rectangle {
			id: item
			width: view.width
			height: itemHeight

			color: "#22000000"
			radius: 5

			required property string textColor
			required property string message


			Label {
				id: lbl

				width: parent.width
				anchors.verticalCenter: parent.verticalCenter

				leftPadding: 2
				rightPadding: 2

				font: Qaterial.Style.textTheme.subtitle2

				color: item.textColor

				text: item.message
				elide: Text.ElideRight
				style: Text.Outline
				styleColor: Qaterial.Colors.black
			}


			/*DropShadow {
				anchors.fill: lbl
				source: lbl
				color: Client.Utils.colorSetAlpha("black", 0.6)
				radius: 1
				spread: 0.5
				samples: 3
				verticalOffset: 1
				horizontalOffset: 1
			}*/

			Timer {
				property int chNum: 0
				running: chNum < 6
				repeat: true
				interval: 300
				onTriggered: {
					++chNum
					if (chNum % 2) {
						lbl.color = item.textColor
					} else {
						lbl.color = Qt.lighter(item.textColor, 1.1)
					}

				}
			}
		}

		snapMode: ListView.SnapToItem

		remove: Transition {
			NumberAnimation { property: "opacity"; to: 0; duration: 250}
		}

		removeDisplaced: Transition {
			NumberAnimation { properties: "x,y"; duration: 600 }
		}
	}


	Timer {
		id: timerRemove
		running: messageModel.count
		repeat: true

		interval: 5000

		onTriggered: messageModel.remove(0)

	}


	function message(text, colorCode) {
		messageModel.append({
								message: text,
								textColor: String(colorCode)
							})

		view.positionViewAtIndex(messageModel.count - 1, ListView.End)
	}
}
