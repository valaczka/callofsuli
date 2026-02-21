import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli
import "JScript.js" as JS


Popup {
	id: root

	visible: false

	modal: true
	focus: true
	closePolicy: Popup.NoAutoClose

	property Item target: null
	property alias title: _title.text
	property alias text: _text.text
	property color dimColor: Qaterial.Style.overlayColor
	property color highlightColor: Qaterial.Style.iconColor()
	property real holeRadius: 12
	property real highlightBorderWidth: 2
	property int delay: 200
	readonly property int arrowSize: 12

	signal dismissed()

	// internal
	property rect _targetRect: Qt.rect(0, 0, 0, 0)

	property Canvas _modalCanvas: null
	property Canvas _arrowCanvas: null


	onTargetChanged: updateGeometry()

	Connections {
		target: root.target
		function onXChanged() { root.updateGeometry() }
		function onYChanged() { root.updateGeometry() }
		function onWidthChanged() { root.updateGeometry() }
		function onHeightChanged() { root.updateGeometry() }
		function onVisibleChanged() { root.updateGeometry() }
	}

	Connections {
		target: Client.mainWindow
		function onXChanged() { root.updateGeometry() }
		function onYChanged() { root.updateGeometry() }
		function onWidthChanged() { root.updateGeometry() }
		function onHeightChanged() { root.updateGeometry() }
	}


	Overlay.modal: Canvas {
		id: canvas

		antialiasing: true

		Component.onCompleted: root._modalCanvas = canvas

		Behavior on opacity
		{
			NumberAnimation { duration: root.delay }
		}

		onPaint: {
			var ctx = getContext("2d")
			ctx.reset()

			ctx.fillStyle = root.dimColor
			ctx.fillRect(0, 0, width, height)

			if (root._targetRect.width <= 0 || root._targetRect.height <= 0)
				return

			// Cut out a rounded-rect hole using destination-out

			var x = root._targetRect.x - root.padding
			var y = root._targetRect.y - root.padding
			var w = root._targetRect.width + 2 * root.padding
			var h = root._targetRect.height + 2 * root.padding
			var r = root.holeRadius

			if (x<0) {
				w += x		// w -= -(x)
				x = 0
			}

			if (y<0) {
				h += y		// h -= -(y)
				y = 0
			}

			w = Math.min(w, Overlay.overlay.width-x)
			h = Math.min(h, Overlay.overlay.height-y)

			if (w <= 0 || h <= 0)
				return

			ctx.globalCompositeOperation = "source-out"

			roundedRectPath(ctx, x, y, w, h, r)
			ctx.fill()

			// 2. spotlight világosítás
			ctx.globalCompositeOperation = "screen"
			ctx.fillStyle = "rgba(255,255,255,0.15)"
			roundedRectPath(ctx, x, y, w, h, r)
			ctx.fill()

			ctx.globalCompositeOperation = "source-over"

			// Draw highlight border
			ctx.strokeStyle = root.highlightColor
			ctx.lineWidth = root.highlightBorderWidth

			roundedRectPath(ctx, x, y, w, h, r)
			ctx.stroke()
		}

		function roundedRectPath(ctx, x, y, w, h, r) {
			r = Math.max(0, Math.min(r, Math.min(w, h) / 2))
			ctx.beginPath()
			ctx.moveTo(x + r, y)
			ctx.arcTo(x + w, y, x + w, y + h, r)
			ctx.arcTo(x + w, y + h, x, y + h, r)
			ctx.arcTo(x, y + h, x, y, r)
			ctx.arcTo(x, y, x + w, y, r)
			ctx.closePath()
		}
	}


	background: Item {
		id: bubble

		Rectangle {
			width: parent.width
			height: parent.height - arrowSize
			y: arrow.isBelow ? arrowSize : 0

			radius: 10
			color: Qaterial.Style.colorTheme.dialog
			border.width: 1
			border.color: root.palette.base
		}

		Canvas {
			id: arrow

			width: 20
			height: arrowSize + 1
			antialiasing: true

			rotation: isBelow ? 0 : 180
			y: isBelow ? 0 : parent.height-height

			property bool isBelow: false

			onPaint: {
				var ctx = getContext("2d")
				ctx.reset()
				ctx.fillStyle = Qaterial.Style.colorTheme.dialog
				ctx.beginPath()
				ctx.moveTo(0, height)
				ctx.lineTo(width / 2, 0)
				ctx.lineTo(width, height)
				ctx.closePath()
				ctx.fill()

				ctx.strokeStyle = root.palette.base
				ctx.lineWidth = 1
				ctx.beginPath()
				ctx.moveTo(0, height)
				ctx.lineTo(width / 2, 0)
				ctx.lineTo(width, height)
				ctx.stroke()
			}

			function reposition(_isBelow) {
				const tr = root._targetRect
				const margin = 12

				isBelow = _isBelow

				let p = Overlay.overlay.mapToItem(bubble, tr.x + tr.width / 2 - width / 2, 0)

				x = Math.max(margin, Math.min(p.x, bubble.width - width - margin))
			}

			Component.onCompleted: {
				root._arrowCanvas = arrow
			}
		}
	}

	implicitWidth: Math.min(Overlay.overlay.width*0.75, Math.max(175, _title.implicitWidth, _text.implicitWidth) + 2*padding)
	implicitHeight: Math.max(150, _button.height + _col.height + 3*padding + arrowSize)


	Column {
		id: _col

		width: parent.width
		anchors.top: parent.top
		anchors.topMargin: _arrowCanvas && _arrowCanvas.isBelow ? arrowSize : 0

		spacing: 3

		Qaterial.LabelHeadline6 {
			id: _title
			width: parent.width

			color: Qaterial.Style.primaryTextColor()

			visible: text.length > 0

			wrapMode: Text.WordWrap
			maximumLineCount: 3
		}

		Qaterial.LabelBody2 {
			id: _text
			width: parent.width

			color: Qaterial.Style.secondaryTextColor()

			visible: text.length > 0

			wrapMode: Text.WordWrap
			maximumLineCount: 3
		}
	}

	QButton {
		id: _button

		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.bottomMargin: _arrowCanvas && _arrowCanvas.isBelow ? 0 : arrowSize

		icon.source: Qaterial.Icons.checkBold
		text: qsTr("Értem")
		onClicked: root.dismissed()
	}





	function updateGeometry() {
		if (!target || !target.visible) {
			_targetRect = Qt.rect(0, 0, 0, 0)
			return
		}

		const p = target.mapToItem(Overlay.overlay, 0, 0)
		_targetRect = Qt.rect(p.x, p.y, target.width, target.height)

		if (_modalCanvas)
			_modalCanvas.requestPaint()

		reposition()
	}

	function reposition() {
		if (!root.visible) return


		const tr = root._targetRect
		const margin = 6
		let isBelow = true

		let px = 0
		let py = 0

		if (tr.width <= 0 || tr.height <= 0) {
			px = Math.round((Overlay.overlay.width - width) / 2)
			py = Math.round(Overlay.overlay.height - height - margin)

		} else {
			const placeBelowY = tr.y + tr.height + margin
			const placeAboveY = tr.y - height - margin

			px = tr.x + tr.width / 2 - width / 2
			px = Math.max(margin, Math.min(px, Overlay.overlay.width - width - margin))

			if (placeBelowY + height <= Overlay.overlay.height - margin)
				py = placeBelowY
			else if (placeAboveY >= margin)  {
				py = placeAboveY
				isBelow = false
			} else
				py = Math.max(margin, Math.min(placeBelowY, Overlay.overlay.height - height - margin))
		}

		let p = parent.mapFromItem(Overlay.overlay, Math.round(px), Math.round(py))

		x = p.x
		y = p.y

		if (_arrowCanvas)
			_arrowCanvas.reposition(isBelow)
	}

	onAboutToShow: updateGeometry()

}
