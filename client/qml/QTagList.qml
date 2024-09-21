import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

Flow {
	id: root
	spacing: 5

	// roles: color, text, textColor

	property var model: []

	property double horizontalPadding: 6 * Qaterial.Style.pixelSizeRatio
	property double verticalPadding: 0

	Repeater {
		model: root.model

		delegate: Rectangle {
			width: _label.implicitWidth+2*root.horizontalPadding
			height: _label.implicitHeight+2*root.verticalPadding
			radius: height/2
			color: modelData.color
			Qaterial.Label {
				id: _label
				anchors.centerIn: parent
				text: modelData.text
				font.pixelSize: Qaterial.Style.textTheme.hint2.pixelSize
				font.family: Qaterial.Style.textTheme.hint2.family
				font.weight: Font.Bold
				color: modelData.textColor ? modelData.textColor: Qaterial.Colors.white
			}
		}
	}

}
