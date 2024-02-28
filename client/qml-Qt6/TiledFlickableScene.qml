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
	property alias world: _world
	property alias joystick: _scene.joystick

	property Item followedItem: null

	default property alias defaultItems: _scene.children

	contentWidth: _container.width
	contentHeight: _container.height

	boundsBehavior: Flickable.DragAndOvershootBounds
	boundsMovement: Flickable.StopAtBounds
	flickableDirection: Flickable.HorizontalAndVerticalFlick

	Item {
		id: _container
		width: Math.max(_scene.width * _scene.scale, flick.width)
		height: Math.max(_scene.height * _scene.scale, flick.height)

		TiledSceneImpl {
			id: _scene

			anchors.horizontalCenter: parent.horizontalCenter
			anchors.verticalCenter: parent.verticalCenter

			running: true

			world: World {
				id: _world
				gravity: Qt.point(0.0, 0.0)
			}

			visibleArea: Qt.rect(flick.contentX / scale, flick.contentY / scale ,
								 flick.contentWidth / scale, flick.contentHeight / scale)


			DebugDraw {
				anchors.fill: parent
				world: _scene.world
				opacity: 0.5
				visible: _scene.debugView
				z: 9999
			}
		}

	}



	SmoothedAnimation {
		id: _animX
		target: flick
		property: "contentX"
		duration: 300
	}

	SmoothedAnimation {
		id: _animY
		target: flick
		property: "contentY"
		duration: 300
	}



	Connections {
		target: followedItem

		function onXChanged() {
			setXOffset()
		}

		function onYChanged() {
			setYOffset()
		}
	}



	function setXOffset() {
		if (!followedItem)
			return

		var fw = flick.width
		var spaceRequired = Math.min(fw*0.3, 250)
		var px = followedItem.x*_scene.scale
		var pw = followedItem.width*_scene.scale
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
		if (!followedItem)
			return

		var fh = flick.height
		var spaceRequired = Math.min(fh*0.3, 250)
		var py = followedItem.y*_scene.scale
		var ph = followedItem.height*_scene.scale
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
