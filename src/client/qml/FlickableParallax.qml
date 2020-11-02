import QtQuick 2.12
import QtQuick.Controls 2.12


Flickable {
	id: root
	anchors.fill: parent

	property Flickable flickableReference: null
	property url source
	property real imageHeight: height*1.5
	property int images: 2

	interactive: false

	contentX: contentWidth*(flickableReference.contentX/flickableReference.contentWidth)
	contentY: flickableReference.contentHeight > height ?
				  contentHeight*(flickableReference.contentY/flickableReference.contentHeight) :
				  contentHeight-height

	contentWidth: rw.implicitWidth
	contentHeight: rw.implicitHeight

	Row {
		id: rw
		Repeater {
			model: root.images

			Image {
				id: img
				source: root.source
				clip: false
				height: root.imageHeight
				width: implicitWidth*(height/implicitHeight)
			}
		}
	}
}
