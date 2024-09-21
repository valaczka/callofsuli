import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	property alias appBar: _appBar
	property alias title: _appBar.title
	property alias subtitle: _appBar.subtitle
	readonly property double paddingTop: _appBar.visible ? _appBar.height : 0
	readonly property bool titleVisible: _appBar.visible && title != "" || subtitle != ""

	default property alias _contentData: _content.data

	Item {
		id: _content

		anchors.fill: parent
		opacity: _appBar.visible ? 0.0 : 1.0

		layer.enabled: true
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
			gradient: titleVisible ? _gradientNormal : _gradientNoTitle
		}
		Rectangle {
			anchors.left: parent.left
			anchors.right: parent.right
			anchors.top: _topRectangle.bottom
			anchors.bottom: parent.bottom
			color: "white"
		}

		Gradient {
			id: _gradientNormal
			orientation: Gradient.Vertical
			GradientStop { position: 0.0; color: "transparent" }
			GradientStop { position: 0.1; color: "#0FFFFFFF" }
			GradientStop { position: 0.65; color: "#19FFFFFF" }
			GradientStop { position: 1.0; color: "white" }
		}

		Gradient {
			id: _gradientNoTitle
			orientation: Gradient.Vertical
			GradientStop { position: 0.0; color: "transparent" }
			GradientStop { position: 0.1; color: "#2FFFFFFF" }
			GradientStop { position: 0.5; color: "#69FFFFFF" }
			GradientStop { position: 1.0; color: "white" }
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

