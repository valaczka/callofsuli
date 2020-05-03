import QtQuick 2.12
import QtQuick.Controls 2.14
import "Style"

ComboBox {
	id: control

	ToolTip.delay: Qt.styleHints.mousePressAndHoldInterval
	ToolTip.visible: hovered && ToolTip.text.length

	property bool sizeToContents: true
	property int modelWidth: 0

	width: (sizeToContents) ? modelWidth + 2*leftPadding + 2*rightPadding : implicitWidth

	TextMetrics {
		id: textMetrics
	}

	onModelChanged: {
		textMetrics.font = control.font
		for(var i = 0; i < model.length; i++){
			textMetrics.text = textAt(i)
			modelWidth = Math.max(textMetrics.width, modelWidth)
		}
	}

}
