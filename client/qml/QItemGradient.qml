import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property alias appBar: _appBar
	property alias title: _appBar.title
	property alias subtitle: _appBar.subtitle
	readonly property double paddingTop: _appBar.visible ? _appBar.height : 0

	default property alias _contentData: _content.data

	Item {
		id: _content

		anchors.fill: parent
		opacity: _appBar.visible ? 0.0 : 1.0
	}

	Item {
		id: _mask
		anchors.fill: _content
		Rectangle {
			id: _topRectangle
			anchors.left: parent.left
			anchors.top: parent.top
			anchors.right: parent.right
			height: root.paddingTop
			gradient: Gradient {
				orientation: Gradient.Vertical
				GradientStop { position: 0.0; color: "transparent" }
				GradientStop { position: 0.2; color: "transparent" }
				GradientStop { position: 0.6; color: "#19FFFFFF" }
				GradientStop { position: 0.9; color: "#66FFFFFF" }
				GradientStop { position: 1.0; color: "white" }
			}
		}
		Rectangle {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: _topRectangle.bottom
			anchors.bottom: parent.bottom
			color: "white"
		}

		visible: false
	}

	OpacityMask {
		anchors.fill: _content
		source: _content
		maskSource: _mask
		visible: _appBar.visible
	}

	AppBar {
		id: _appBar
		width: parent.width
		anchors.left: parent.left
		anchors.top: parent.top

		background: null
	}
}

