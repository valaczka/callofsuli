import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS

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
		anchors.leftMargin: 10
		anchors.rightMargin: 10
		anchors.topMargin: 5
		anchors.bottomMargin: 5

		clip: true

		model: ListModel {
			id: messageModel
		}

		delegate: Item {
			width: view.width
			height: itemHeight

			Label {
				id: lbl
				visible: false
				width: parent.width
				anchors.verticalCenter: parent.verticalCenter

				font.pixelSize: (Qt.platform.os === "android" ? 12 : 16)
				font.weight: Font.DemiBold

				property int colorCodePrivate: textColor

				color: switch (colorCodePrivate) {
					   case 1:
						   CosStyle.colorPrimaryLighter
						   break
					   case 2:
						   CosStyle.colorWarningLighter
						   break
					   case 3:
						   CosStyle.colorErrorLighter
						   break
					   case 10:
						   CosStyle.colorAccentLighter
						   break
					   case 11:
						   CosStyle.colorPrimaryLight
						   break
					   case 12:
						   CosStyle.colorWarningLight
						   break
					   case 13:
						   CosStyle.colorErrorLight
						   break
					   default:
						   "white"
						   break
					   }

				text: message
				elide: Text.ElideRight
			}

			Glow {
				anchors.fill: lbl
				source: lbl
				color: "black"
				radius: 2
				spread: 0.5
				samples: 5
			}

			Timer {
				property int chNum: 0
				running: chNum < 6
				repeat: true
				interval: 300
				onTriggered: {
					chNum++
					if (lbl.colorCodePrivate >= 10)
						lbl.colorCodePrivate -= 10
					else
						lbl.colorCodePrivate += 10
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

		add: Transition {
			NumberAnimation { property: "opacity"; from: 0; to: 1; duration: 250}
		}
	}


	Timer {
		id: timerRemove
		running: messageModel.count > 1
		repeat: true

		interval: 3000

		onTriggered: messageModel.remove(0)

	}


	function message(text, colorCode) {
		messageModel.append({
								message: text,
								textColor: colorCode
							})

		view.positionViewAtIndex(messageModel.count - 1, ListView.End)
	}
}
