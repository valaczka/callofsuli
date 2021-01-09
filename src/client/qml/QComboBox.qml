import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"
import "JScript.js" as JS

ComboBox {
	id: control

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	font.pixelSize: CosStyle.pixelSize

	property bool sizeToContents: true
	property int modelWidth: 0
	readonly property bool modelIsArray: Array.isArray(model)

	implicitWidth: sizeToContents ? (modelWidth + leftPadding + rightPadding + indicator.width) : 400
	implicitHeight: Math.max(CosStyle.pixelSize*2, 48)

	TextMetrics {
		id: textMetrics
	}

	readonly property int pxs: CosStyle.pixelSize

	onPxsChanged: recalculate()

	onModelChanged: recalculate()

	function recalculate() {
		var w = 50
		textMetrics.font = control.font
		if (Array.isArray(model)) {
			for(var i = 0; i < model.length; i++){
				textMetrics.text = model[i][textRole]
				w = Math.max(textMetrics.width, w)
			}
		} else {
			for(i = 0; i < model.count; i++){
				textMetrics.text = control.textAt(i)
				w = Math.max(textMetrics.width, w)
			}
		}

		modelWidth = w
	}

	delegate: ItemDelegate {
		width: control.width
		contentItem: Text {
			text: modelIsArray ? model.modelData[textRole] : model[textRole]
			color: CosStyle.colorPrimaryLighter
			font: control.font
			elide: Text.ElideRight
			verticalAlignment: Text.AlignVCenter
		}
		highlighted: control.highlightedIndex === index
	}

	popup: Popup {
		y: control.height - 1
		width: control.width
		implicitHeight: contentItem.implicitHeight
		padding: 1

		contentItem: ListView {
			clip: true
			implicitHeight: contentHeight
			model: control.popup.visible ? control.delegateModel : null
			currentIndex: control.highlightedIndex

			ScrollIndicator.vertical: ScrollIndicator { }
		}

		background: Rectangle {
			radius: 2
			color: JS.setColorAlpha(Qt.darker(CosStyle.colorPrimaryDark,2.5), 0.95)
		}
	}

}
