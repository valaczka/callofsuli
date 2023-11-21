import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import Box2D 2.0
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

/*

  z layers:

  0-4: TiledLayers
  5: Ladders
  6-8: Objects
  9: Enemies
  10: Player
  11-15: TiledLayers
  16: Crosshair

  */

GameSceneImpl {
	id: control

	property real _sceneScale: 1.0
	readonly property real _sceneZoom: game && game.pageItem ?
										   Math.max(Math.min(game.pageItem.width/control.width, game.pageItem.height/control.height, 0.7), 0.35) :
										   1.0


	y: parent.height-(control.height*control.scale)

	scale: _sceneZoom+((1.0-_sceneZoom)*_sceneScale)
	transformOrigin: Item.TopLeft

	world: World {
		//timeStep: 1.0/25.0
	}

	mouseArea: area

	layer.enabled: true


	Image {
		anchors.fill: parent
		fillMode: Image.Pad
		source: control.imageTerrain
		visible: control.imageTerrain != ""
	}


	Image {
		anchors.fill: parent
		fillMode: Image.Pad
		source: control.imageOver
		visible: control.imageOver != ""
		z: 11
	}


	// Base ground

	GameObject {
		x: 0
		y: control.height
		width: control.width
		height: 10

		body.fixtures: Box {
			width: control.width
			height: 10
			density: 1
			restitution: 0
			friction: 1
			categories: Box.Category1
			collidesWith: (Box.Category2|Box.Category5)
			readonly property bool baseGround: true
		}

		Component.onCompleted: bodyComplete()
	}


	MouseArea {
		id: area
		anchors.fill: parent
		hoverEnabled: true
	}


	DebugDraw {
		id: debugDraw
		anchors.fill: parent
		world: control.world
		opacity: 0.5
		visible: control.debugView
	}


	Connections {
		target: game

		function onPlayerChanged() {
			showPlayerLocator()
		}
	}

	onZoomOverviewChanged: if (zoomOverview)
							   showPlayerLocator()



	Component {
		id: playerLocatorComponent
		GamePlayerLocator { }
	}

	function showPlayerLocator() {
		if (game.player) {
			var r = playerLocatorComponent.createObject(control)
			r.anchors.centerIn = game.player
		}
	}
}
