import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial


QIconLabel {
	id: root

	topPadding: 20 * Qaterial.Style.pixelSizeRatio

	horizontalAlignment: Qt.AlignLeft
	font: Qaterial.Style.textTheme.body2Upper
	wrapMode: Text.Wrap
}
