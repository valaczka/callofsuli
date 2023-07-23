import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS
import QtCharts 2.15

ChartView {
	id: _chart

	width: 350 * Qaterial.Style.pixelSizeRatio
	height: 150 * Qaterial.Style.pixelSizeRatio
	antialiasing: true
	animationOptions: ChartView.SeriesAnimations
	backgroundColor: "transparent"
	backgroundRoundness: 0
	legend.visible: false

	margins.top: 0
	margins.bottom: 0
	margins.left: 0
	margins.right: 0


	onWidthChanged: setRange()

	BarSeries {
		BarSet {
			id: _barSet
			color: Qaterial.Style.iconColor()
			borderWidth: 0
		}

		axisY: ValueAxis {
			id: _yAxis
			min: 0
			max: 500

			labelFormat: "%d XP"

			gridLineColor: Qaterial.Style.dividersColor()
			labelsFont.family: Qaterial.Style.textTheme.caption.family
			labelsFont.pixelSize: Qaterial.Style.textTheme.caption.pixelSize
			labelsFont.weight: Font.Bold
			labelsColor: Qaterial.Style.secondaryTextColor()

			tickCount: 5
			minorTickCount: 1

			minorGridLineColor: Qaterial.Style.disabledDividersColor()
			minorGridVisible: true
			lineVisible: false

			shadesVisible: false

		}

		axisX: BarCategoryAxis {
			id: _xAxis

			gridLineColor: Qaterial.Style.disabledDividersColor()
			labelsFont.family: Qaterial.Style.textTheme.caption.family
			labelsFont.pixelSize: Qaterial.Style.textTheme.caption.pixelSize
			labelsFont.weight: Font.Bold
			labelsColor: Qaterial.Style.secondaryTextColor()

			minorGridVisible: false
			lineVisible: false
			shadesVisible: false

		}
	}


	Flickable {
		id: _flickable
		anchors.fill: parent

		contentHeight: parent.height
		contentWidth: parent.width*3
		flickableDirection: Flickable.HorizontalFlick

		boundsBehavior: Flickable.StopAtBounds
		boundsMovement: Flickable.StopAtBounds

		property real _oldX: 0
		property bool _activated: false

		onContentXChanged: {
			if (_activated) {
				_chart.animationOptions = ChartView.NoAnimation
				_chart.scrollRight(contentX-_oldX)
			}
			_oldX = contentX

		}
	}


	function loadList(list) {
		let diff = 14

		for (let i=0; i<list.length; ++i) {
			let d = list[i]

			if (Number(d.diff) > diff)
				diff = Number(d.diff)

			if (Number(d.xp) > _yAxis.max)
				_yAxis.max = Number(d.xp)
		}

		let zeroDay = new Date()
		zeroDay.setDate(zeroDay.getDate() - diff)

		let model = []
		let values = []

		for (let i=0; i<=diff; ++i) {
			let c = JS.readableDateDate(zeroDay, "MM.dd.", "yy.MM.dd.")
			model.push(c)
			zeroDay.setDate(zeroDay.getDate()+1)
		}

		for (let i=0; i<list.length; ++i) {
			let d = list[i]

			values.push(Qt.point(diff - Number(d.diff), d.xp))
		}

		_chart.animationOptions = ChartView.SeriesAnimations

		_xAxis.categories = model
		_barSet.values = values

		setRange()
	}

	function setRange() {
		if (!_xAxis.categories.length)
			return

		let colWidth = 55 * Qaterial.Style.pixelSizeRatio
		let n = Math.floor(plotArea.width/colWidth)

		_xAxis.max = _xAxis.categories[_xAxis.categories.length-1]
		_xAxis.min = _xAxis.categories[Math.max(0, _xAxis.categories.length-n)]

		_flickable._activated = false
		_flickable.contentWidth = colWidth * (_xAxis.categories.length+3)
		_flickable.contentX = _flickable.contentWidth - _flickable.width
		_flickable._activated = true
	}
}
