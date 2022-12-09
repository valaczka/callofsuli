import QtQuick 2.7
import QtGraphicalEffects 1.0
import "Style"

Item {
	id: control

	implicitHeight: 100
	implicitWidth: 100

	property alias source: img.source
	property int rankId: -1
	property string rankImage: ""
	property bool active: true

	readonly property real _size: Math.min(width/1.2, height)

	Item {
		id: realContent
		anchors.centerIn: parent

		width: _size*1.2
		height: _size

		Image {
			id: img
			width: _size
			height: _size
			fillMode: Image.PreserveAspectFit
			visible: false
			smooth: true
			asynchronous: true
			cache: true
		}


		OpacityMask {
			anchors.fill: img
			visible: active && img.status == Image.Ready
			source: img
			maskSource: mask
		}

		Rectangle {
			id: mask
			visible: active && opacity && img.source && img.source != ""
			opacity: img.status != Image.Ready ? 1.0 : 0.0
			width: _size
			height: _size
			radius: _size/2
			color: "grey"

			Behavior on opacity {
				NumberAnimation { duration: 125; easing.type: Easing.OutQuad }
			}
		}

		Image {
			id: rankImg

			visible: active && (rankId > 0 || rankImage != "")

			source: visible ? cosClient.rankImageSource(rankId, -1, rankImage) : ""
			fillMode: Image.PreserveAspectFit

			width: img.source && img.source != "" ? _size*0.5 : _size
			height: width

			anchors.right: img.right
			anchors.bottom: img.bottom
			anchors.rightMargin: img.source && img.source != "" ? -_size*0.2 : 0
		}

		QFontImage {
			visible: !active
			size: _size
			icon: CosStyle.iconUserWhite
			color: "#99ffffff"
		}

	}
}
