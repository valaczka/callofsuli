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
			id: _seriesEnemies

			color: Qaterial.Colors.red600

			axisY: _yAxis
			axisX: _xAxis

			borderWidth: 0
			markerSize: 9
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

		readonly property rect _sceneRect: game && game.currentScene ? game.currentScene.visibleArea : Qt.rect(0,0,0,0)
		readonly property real _scaleX: game && game.currentScene ? _view.width/game.currentScene.width : 0
		readonly property real _scaleY: game && game.currentScene ? _view.height/game.currentScene.height : 0

		readonly property real _sceneX: _sceneRect.x * _scaleX
		readonly property real _sceneY: _sceneRect.y * _scaleY

		x: _view.x + _view.plotArea.x + Math.max(_sceneX, 0)
		y: _view.y + _view.plotArea.y + Math.max(_sceneY, 0)
		width: Math.min(_sceneRect.width * _scaleX, _view.plotArea.width-_sceneX)
		height: Math.min(_sceneRect.height * _scaleY, _view.plotArea.height-_sceneY)
	}

	Connections {
		target: game

		function onCurrentSceneChanged() {
			if (!game.currentScene)
				return

			_xAxis.max = game.currentScene.width
			_xAxis.min = 0
			_yAxis.max = game.currentScene.height
			_yAxis.min = 0
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
		}
	}
}
