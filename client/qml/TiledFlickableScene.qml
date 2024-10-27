import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import Box2D
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Flickable {
	id: flick

	property alias scene: _scene

	default property alias defaultItems: _scene.children

	contentWidth: _scene.viewport.width * _scene.scale
	contentHeight: _scene.viewport.height * _scene.scale

	anchors.centerIn: parent
	width: parent ? Math.min(_container.width, parent.width) : _container.width
	height: parent ? Math.min(_container.height, parent.height) : _container.height

	visible: _scene.game && _scene.game.currentScene == _scene

	boundsBehavior: Flickable.DragAndOvershootBounds
	boundsMovement: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick

	property bool _interactiveDisabled: false

	interactive: _scene.game && _scene.game.flickableInteractive && !_interactiveDisabled

	readonly property real minZoom: _scene.viewport.width > 0 && _scene.viewport.height > 0 && parent ?
							   Math.min(1.0, Math.max(0.2,
													  flick.parent.width/_scene.viewport.width,
													  flick.parent.height/_scene.viewport.height,
													  )) :
							   0.2
	readonly property real maxZoom: 2.5
	readonly property real zoomStep: 0.2

	Item {
		id: _container
		width: _scene.width * _scene.scale
		height: _scene.height * _scene.scale
		x: -_scene.viewport.x * _scene.scale
		y: -_scene.viewport.y * _scene.scale

		TiledSceneImpl {
			id: _scene

			property real prevScale: 1.0

			anchors.centerIn: parent
			transformOrigin: Item.Center

			scale: game ? game.baseScale : 1.0

			onScaleChanged: {
				if ((width * scale) > flick.width) {
					var xoff = (flick.width / 2 + flick.contentX) * scale / prevScale;
					flick.contentX = xoff - flick.width / 2
				}
				if ((height * scale) > flick.height) {
					var yoff = (flick.height / 2 + flick.contentY) * scale / prevScale;
					flick.contentY = yoff - flick.height / 2
				}
				prevScale=scale;
			}

			onScaleResetRequest: {
				scale = game ? game.baseScale : 1.0
				prevScale = scale

				flick.returnToBounds();

				setXOffset()
				setYOffset()
			}


			function zoomIn() {
				_scene.scale = Math.min(_scene.scale * (1.0+zoomStep), flick.maxZoom)
				flick.returnToBounds();
			}

			function zoomOut() {
				_scene.scale = Math.max(_scene.scale * (1.0-zoomStep), flick.minZoom)
				flick.returnToBounds();
			}

			visibleArea: flick.visible ? Qt.rect(0, 0, _scene.width, _scene.height) :
										 Qt.rect(0,0,0,0)

			onScreenArea: flick.visible ? Qt.rect((flick.contentX - _container.x) / _scene.scale,
												  (flick.contentY - _container.y) / _scene.scale ,
												 flick.width / _scene.scale, flick.height / _scene.scale) :
										 Qt.rect(0,0,0,0)

			//------------------------------------------------
			/*onTestPointsChanged: _canvas.requestPaint()

			Canvas {
				id: _canvas
				anchors.fill: parent
				z: 9998
				onPaint: {
					let ctx = _canvas.getContext("2d")
					ctx.save()
					ctx.clearRect(0, 0, _canvas.width, _canvas.height)
					ctx.strokeStyle = Qaterial.Colors.pink300
					ctx.lineWidth = 2;
					ctx.beginPath();

					for (let i=0; i<_scene.testPoints.length; ++i) {
						let p=_scene.testPoints[i]
						if (i==0)
							ctx.moveTo(p.x, p.y)
						else
							ctx.lineTo(p.x, p.y)
					}

					ctx.stroke();

					ctx.restore();
				}
			}*/
			//------------------------------------------------
		}

		TiledSceneEffectSnow {
			visible: _scene.sceneEffect == TiledSceneDefinition.EffectSnow
			anchors.fill: _container
		}

		TiledSceneEffectRain {
			visible: _scene.sceneEffect == TiledSceneDefinition.EffectRain
			anchors.fill: _container
		}

		DebugDraw {
			anchors.fill: _scene
			world: _scene.world
			opacity: 0.5
			visible: _scene.game && _scene.game.debugView
			scale: _scene.scale
		}
	}

	PinchArea {
		anchors.fill: parent

		enabled: _scene.game && !_scene.game.joystick.hasTouch

		pinch.target: _scene
		pinch.minimumScale: flick.minZoom
		pinch.maximumScale: flick.maxZoom

		onPinchStarted: {
			flick._interactiveDisabled = true
		}

		onPinchUpdated: pinch => {
							flick.contentX += pinch.previousCenter.x - pinch.center.x
							flick.contentY += pinch.previousCenter.y - pinch.center.y
						}

		onPinchFinished: {
			flick._interactiveDisabled = false
			flick.returnToBounds()
		}

		MouseArea {								// Workaround (https://bugreports.qt.io/browse/QTBUG-77629)
			anchors.fill: parent

			onClicked: event => {
						   if (_scene.game) {
							   _scene.game.onMouseClick(_scene.viewport.x + (event.x / _scene.scale),
														_scene.viewport.y + (event.y / _scene.scale),
														event.modifiers)
						   }
					   }

			onWheel: wheel => {
						 if (wheel.modifiers & Qt.ControlModifier) {
							 if (wheel.angleDelta.y > 0) {
								 _scene.zoomIn()
							 } else if (wheel.angleDelta.y < 0) {
								 _scene.zoomOut()
							 }
						 } else if ((wheel.modifiers & Qt.ShiftModifier) || (wheel.modifiers & Qt.AltModifier)) {
							 if (wheel.angleDelta.y > 0) {
								 flick.contentX -= flick.contentWidth * flick.visibleArea.widthRatio*0.1
							 } else if (wheel.angleDelta.y < 0) {
								 flick.contentX += flick.contentWidth * flick.visibleArea.widthRatio*0.1
							 }
							 flick.returnToBounds()
						 } else {
							 if (wheel.angleDelta.y > 0) {
								 flick.contentY -= flick.contentHeight * flick.visibleArea.heightRatio*0.1
							 } else if (wheel.angleDelta.y < 0) {
								 flick.contentY += flick.contentHeight * flick.visibleArea.heightRatio*0.1
							 }
							 flick.returnToBounds()
						 }
					 }
		}
	}


	SmoothedAnimation {
		id: _animX
		target: flick
		property: "contentX"
		duration: 250
	}

	SmoothedAnimation {
		id: _animY
		target: flick
		property: "contentY"
		duration: 250
	}


	Connections {
		target: _scene.game

		function onFollowedItemChanged() {
			setXOffset()
			setYOffset()
		}
	}

	Connections {
		target: _scene.game ? _scene.game.followedItem : null

		function onXChanged() {
			setXOffset()
		}

		function onYChanged() {
			setYOffset()
		}
	}

	onWidthChanged: setXOffset()
	onHeightChanged: setYOffset()


	function setXOffset() {
		if (!_scene.game || !_scene.game.followedItem || _scene.game.followedItem.scene != _scene)
			return

		var fw = flick.width
		var px = _scene.game.followedItem.x*_scene.scale + _container.x
		var pw = _scene.game.followedItem.width*_scene.scale
		var spaceRequired = Math.min((fw-pw)*0.45, 500)
		var cx = flick.contentX
		var cw = flick.contentWidth
		var x = -1

		if (px-spaceRequired < cx) {
			x = px-spaceRequired

			if (x<0)
				x = 0
		} else if (px+pw+spaceRequired > (cx+fw)) {
			x = px+pw+spaceRequired-fw

			if (x+fw > cw)
				x = cw-fw
		}

		if (x>-1)
			animateX(x)
	}


	function setYOffset() {
		if (!_scene.game || !_scene.game.followedItem || _scene.game.followedItem.scene != _scene)
			return

		var fh = flick.height
		var py = _scene.game.followedItem.y*_scene.scale + _container.y
		var ph = _scene.game.followedItem.height*_scene.scale
		var spaceRequired = Math.min((fh-ph)*0.45, 500)
		var cy = flick.contentY
		var ch = flick.contentHeight
		var y = -1


		if (py-spaceRequired < cy) {
			y = py-spaceRequired

			if (y<0)
				y = 0
		} else if (py+ph+spaceRequired > (cy+fh)) {
			y = py+ph+spaceRequired-fh

			if (y+fh > ch)
				y = ch-fh
		}

		if (y>-1)
			animateY(y)
	}



	function animateX(_x) {
		if (_animX.running || Math.abs(flick.contentX-_x) > 50) {
			_animX.to = _x
			_animX.restart()
		} else {
			flick.contentX = _x
		}
	}

	function animateY(_y) {
		if (_animY.running || Math.abs(flick.contentY-_y) > 50) {
			_animY.to = _y
			_animY.restart()
		} else {
			flick.contentY = _y
		}
	}

}
