import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import QtCharts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Rectangle {
	id: root

	property RpgGameImpl game: null
	property alias view: _view

	color: Client.Utils.colorSetAlpha(Qaterial.Colors.black, 0.85)


	ChartView {
		id: _view
		anchors.fill: parent

		antialiasing: true
		animationOptions: ChartView.NoAnimation
		backgroundColor: "transparent"
		backgroundRoundness: 0
		legend.visible: false

		margins.top: 0
		margins.bottom: 0
		margins.left: 0
		margins.right: 0

		axes: [
			ValueAxis {
				id: _xAxis

				gridLineColor: Qaterial.Style.dividersColor()

				tickCount: 5
				minorTickCount: 1

				minorGridVisible: false
				lineVisible: false
				labelsVisible: false
				shadesVisible: false
			},

			ValueAxis {
				id: _yAxis

				gridLineColor: Qaterial.Style.dividersColor()

				tickCount: 5
				minorTickCount: 1

				minorGridVisible: false
				lineVisible: false
				labelsVisible: false
				shadesVisible: false

			}
		]

		ScatterSeries {
			id: _seriesPoints

			color: Qaterial.Colors.green400

			axisY: _yAxis
			axisX: _xAxis

			borderWidth: 0
			markerSize: 10
		}

		ScatterSeries {
			id: _seriesEnemies

			color: Qaterial.Colors.red600

			axisY: _yAxis
			axisX: _xAxis

			borderWidth: 0
			markerSize: 11
		}

		ScatterSeries {
			id: _seriesPlayer

			color: Qaterial.Colors.amber600

			axisY: _yAxis
			axisX: _xAxis

			borderColor: Qaterial.Colors.amber200
		}

	}


	Rectangle {
		border.color: Qaterial.Colors.cyan400
		border.width: 2
		color: "transparent"

		readonly property rect _sceneRect: game && game.currentScene ? game.currentScene.onScreenArea : Qt.rect(0,0,0,0)

		x: game && game.currentScene
		   ? _view.x + _view.plotArea.x + ((_sceneRect.x - game.currentScene.viewport.x) / game.currentScene.viewport.width) * _view.plotArea.width
		   : 0
		y: game && game.currentScene
		   ? _view.y + _view.plotArea.y + ((_sceneRect.y - game.currentScene.viewport.y) / game.currentScene.viewport.height) * _view.plotArea.height
		   : 0

		width: game && game.currentScene
			   ? (_sceneRect.width / game.currentScene.viewport.width) * _view.plotArea.width
			   : 100

		height: game && game.currentScene
				? (_sceneRect.height / game.currentScene.viewport.height) * _view.plotArea.height
				: 100
	}

	Connections {
		target: game

		function onCurrentSceneChanged() {
			if (!game.currentScene)
				return

			_xAxis.max = game.currentScene.viewport.x + game.currentScene.viewport.width
			_xAxis.min = game.currentScene.viewport.x
			_yAxis.max = game.currentScene.height - game.currentScene.viewport.y
			_yAxis.min = game.currentScene.height - (game.currentScene.viewport.y + game.currentScene.viewport.height)
		}
	}

	MouseArea {
		anchors.fill: parent
		acceptedButtons: Qt.LeftButton
		onClicked: root.visible = false
	}

	Component.onCompleted: {
		if (game) {
			game.scatterSeriesEnemies = _seriesEnemies
			game.scatterSeriesPlayers = _seriesPlayer
			game.scatterSeriesPoints = _seriesPoints
		}
	}
}
