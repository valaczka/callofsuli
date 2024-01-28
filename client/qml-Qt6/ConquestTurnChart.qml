import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qt5Compat.GraphicalEffects
import QtCharts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
	id: root

	property ConquestGame game: null

	width: 150 * Qaterial.Style.pixelSizeRatio
	height: 150 * Qaterial.Style.pixelSizeRatio

	radius: Math.min(width, height)/2

	color: Client.Utils.colorSetAlpha(Qaterial.Style.dialogColor, 0.5)

	ChartView {
		anchors.fill: parent
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

	onGameChanged: {
		if (game) {
			game.configChanged.connect(reload)
			game.currentStageChanged.connect(function(){
				_series.clear()
				reload()
			})
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
			o.borderWidth = 2
			o.color = game.getPlayerColor(list[i].player)

			if (!active)
				o.color = Qt.darker(o.color, 3.0)
		}
	}

}
