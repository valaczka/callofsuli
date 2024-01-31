import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import SortFilterProxyModel
import Qt.labs.animation
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


Item {
	id: root

	property ConquestGame game: null
	property Flickable flickable: null

	property alias transformTranslate: _sceneTranslate
	property alias transformRotation: _sceneRotation


	signal zoomPerformed()

	transformOrigin: Item.Center

	property real _prevScale: 1.0
	property bool _autoContentPosition: true

	onScaleChanged: {
		if (!flickable || !_autoContentPosition)
			return

		if ((width * scale) > flickable.width) {
			var xoff = (flickable.width / 2 + flickable.contentX) * scale / _prevScale;
			flickable.contentX = xoff - flickable.width / 2
		}
		if ((height * scale) > flickable.height) {
			var yoff = (flickable.height / 2 + flickable.contentY) * scale / _prevScale;
			flickable.contentY = yoff - flickable.height / 2
		}
		_prevScale=scale;
	}

	BoundaryRule on scale {
		minimum: 0.3
		maximum: 5.0
	}


	width: game ? game.worldSize.width : implicitWidth
	height: game ? game.worldSize.height : implicitHeight


	Image {
		source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/bg.png" : ""
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
	}

	Repeater {
		model: game ? game.landDataList : null

		delegate: ConquestLand {
			id: _land
			landData: model.qtObject
			on_PickedChanged: {
				if (_picked)
					zoomToLand(_land)
			}
		}
	}

	Image {
		source: game && game.config.world.name != "" ? "qrc:/conquest/"+game.config.world.name+"/over.png" : ""
		anchors.fill: parent
		fillMode: Image.PreserveAspectFit
	}


	transform: [
		Rotation {
			id: _sceneRotation
			origin.x: width/2
			origin.y: height  ///2
			axis.x: 1
			axis.y: 0
			axis.z: 0
		},
		Translate {
			id: _sceneTranslate
		}
	]


	Connections {
		target: game

		function onMapDownRequest() {
			_animationZoom.stop()
		}

		function onMapUpRequest() {
			_animationZoom.stop()
		}
	}

	SequentialAnimation {
		id: _animationZoom
		alwaysRunToEnd: false

		property real zoomScale: 1.0
		property real zoomContentX: 0
		property real zoomContentY: 0


		ScriptAction {
			script: {
				root._autoContentPosition = false
			}
		}

		ParallelAnimation {
			PropertyAnimation {
				target: root
				property: "scale"
				to: _animationZoom.zoomScale
				duration: 750
				easing.type: Easing.InOutQuad
			}
			PropertyAnimation {
				target: flickable
				property: "contentX"
				to: _animationZoom.zoomContentX
				duration: 750
				easing.type: Easing.InOutQuad
			}
			PropertyAnimation {
				target: flickable
				property: "contentY"
				to: _animationZoom.zoomContentY
				duration: 750
				easing.type: Easing.InOutQuad
			}
		}

		ScriptAction {
			script: {
				root._autoContentPosition = true
			}
		}
	}

	function zoomToLand(_item) {
		if (!_item)
			return

		let s = Math.min(Math.min(flickable.width/_item.width, flickable.height/_item.height)*0.9, 2.5)

		_animationZoom.zoomScale = s
		_animationZoom.zoomContentX = _item.x*s - (flickable.width-_item.width*s)/2
		_animationZoom.zoomContentY = _item.y*s - (flickable.height-_item.height*s)/2
		_animationZoom.start()

		zoomPerformed()
	}
}
