import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import QtGraphicalEffects 1.15
import QtCharts 2.15

Rectangle {
	id: root

	property ConquestGame game: null

	property real size: 90 * Qaterial.Style.pixelSizeRatio

	width: size
	height: size

	radius: Math.min(width, height)/2

	color: Client.Utils.colorSetAlpha(Qaterial.Style.dialogColor, 0.5)

	ChartView {
		id: _view
		width: parent.width
		height: parent.height
		antialiasing: true
		animationOptions: ChartView.SeriesAnimations
		backgroundColor: "transparent"
		backgroundRoundness: 0
		legend.visible: false

		margins.top: 0
		margins.bottom: 0
		margins.left: 0
		margins.right: 0

		PieSeries {
			id: _series
			holeSize: 0.6
			size: 0.85
		}
	}


	SequentialAnimation {
		id: _markAnimation
		alwaysRunToEnd: true


		PropertyAction {
			target: _view
			property: "animationOptions"
			value: ChartView.NoAnimation
		}

		PropertyAnimation {
			target: root
			properties: "width, height"
			to: size*1.6
			duration: 350
		}

		PauseAnimation {
			duration: 2500
		}

		PropertyAnimation {
			target: root
			properties: "width, height"
			to: size
			duration: 650
		}

		PropertyAction {
			target: _view
			property: "animationOptions"
			value: ChartView.SeriesAnimations
		}
	}

	function reload() {
		let list = game ? game.config.turnList : []

		if (list.length !== _series.count)
			_series.clear()

		for (let i=0; i<list.length; ++i) {
			let o = null

			if (i < _series.count)
				o = _series.at(i)
			else
				o = _series.append("-", 1)

			let active = (game.config.currentTurn === i)

			o.exploded = active
			o.borderColor = Qaterial.Colors.black
			o.borderWidth = 1
			o.color = game.getPlayerColor(list[i].player)

			if (game.config.currentTurn > i)
				o.color = Qaterial.Colors.gray900
			else if (game.config.currentTurn < i)
				o.color = Qt.darker(o.color, 2.5)
		}
	}


	function reloadStage() {
		_markAnimation.start()
		_series.clear()
		reload()
	}



	Component.onCompleted: {
		if (game) {
			game.configChanged.connect(reload)
			game.currentStageChanged.connect(reloadStage)
		}
	}

	Component.onDestruction: {
		if (game) {
			game.configChanged.disconnect(reload)
			game.currentStageChanged.disconnect(reloadStage)
		}
	}
}
