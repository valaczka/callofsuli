import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Rectangle {
	id: root

	property int num: 0
	property color textColor: Qaterial.Colors.white

	property double horizontalPadding: 1 * Qaterial.Style.pixelSizeRatio
	property double defaultSize: 6 * Qaterial.Style.pixelSizeRatio
	property double pixelSize: Qaterial.Style.textTheme.hint1.pixelSize

	width: Math.max(defaultSize, _label.implicitWidth, _label.implicitHeight)+2*root.horizontalPadding
	height: width
	radius: width/2

	Qaterial.Label {
		id: _label
		anchors.centerIn: parent
		text: root.num < 100 ? root.num : "99+"
		font.pixelSize: root.pixelSize
		font.family: Qaterial.Style.textTheme.hint1.family
		font.weight: Font.Bold
		color: root.textColor
	}
}
