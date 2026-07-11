import QtQuick
import CallOfSuli


TiledEffectFogImpl {
	id: root

	property color tintColor: "#ffffff"

	layer.enabled: true
	layer.smooth: true

	layer.effect: ShaderEffect {
		property vector2d itemSize: Qt.vector2d(root.width, root.height)
		property real fadeWidth: 35
		property color tintColor: root.tintColor

		fragmentShader: "qrc:/shaders/ellipseFade.frag.qsb"
	}
}
