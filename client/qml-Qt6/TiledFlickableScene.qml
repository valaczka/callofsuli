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

	contentWidth: _container.width
	contentHeight: _container.height

	anchors.fill: parent

	visible: _scene.game && _scene.game.currentScene == _scene

	boundsBehavior: Flickable.DragAndOvershootBounds
	boundsMovement: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick

	property real minZoom: 0.2
	property real maxZoom: 2.5
	property real zoomStep: 0.2

	Item {
		id: _container
		width: Math.max(_scene.width * _scene.scale, flick.width)
		height: Math.max(_scene.height * _scene.scale, flick.height)


		TiledSceneImpl {
			id: _scene

			property real prevScale: 1.0

			anchors.centerIn: parent
			transformOrigin: Item.Center


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


			function zoomIn() {
				if (_scene.scale<flick.maxZoom)
					_scene.scale *= (1.0+zoomStep)
				else
					_scene.scale = flick.maxZoom
				flick.returnToBounds();
			}
			function zoomOut() {
				if (_scene.scale>flick.minZoom)
					_scene.scale *= (1.0-zoomStep)
				else
					_scene.scale = flick.minZoom;
				flick.returnToBounds();
			}

			visibleArea: flick.visible ? Qt.rect(flick.contentX / scale, flick.contentY / scale ,
												 flick.width / scale, flick.height / scale) :
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
			flick.interactive = false
		}

		onPinchUpdated: pinch => {
			flick.contentX += pinch.previousCenter.x - pinch.center.x
			flick.contentY += pinch.previousCenter.y - pinch.center.y
		}

		onPinchFinished: {
			flick.interactive = true
			flick.returnToBounds()
		}

		MouseArea {								// Workaround (https://bugreports.qt.io/browse/QTBUG-77629)
			anchors.fill: parent

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
		var px = _scene.game.followedItem.x*_scene.scale
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
		var py = _scene.game.followedItem.y*_scene.scale
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


	function setOffsetTo(_x, _y) {
		var fh = flick.height
		var py = _y*_scene.scale
		var ch = flick.contentHeight
		var y = py-fh/2

		if (y<0)
			y = 0

		if (y+fh > ch)
			y = ch-fh

		animateY(y)

		var fw = flick.width
		var px = _x*_scene.scale
		var cw = flick.contentWidth
		var x = px-fw/2

		if (x<0)
			x = 0
		if (x+fw > cw)
			x = cw-fw

		animateX(x)
	}

}
