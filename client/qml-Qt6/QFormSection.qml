import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial


QIconLabel {
	id: root

	topPadding: 20 * Qaterial.Style.pixelSizeRatio

	horizontalAlignment: Qt.AlignLeft
	font: Qaterial.Style.textTheme.body2Upper
	wrapMode: Text.Wrap
}
