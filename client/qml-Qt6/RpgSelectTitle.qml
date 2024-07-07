import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

QIconLabel {
	id: control

	topPadding: 20 * Qaterial.Style.pixelSizeRatio
	bottomPadding: 20 * Qaterial.Style.pixelSizeRatio
	anchors.leftMargin: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
	width: parent.width
		   -Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
		   -Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)
	horizontalAlignment: Qt.AlignLeft
	font: Qaterial.Style.textTheme.headline5
	icon.width: 1.5 * font.pixelSize
	icon.height: 1.5 * font.pixelSize
	wrapMode: Text.Wrap
}
