import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

QIconLabel {
	id: control

	topPadding: 10 * Qaterial.Style.pixelSizeRatio
	bottomPadding: 10 * Qaterial.Style.pixelSizeRatio
	/*anchors.leftMargin: Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
	width: parent.width
		   -Math.max(Client.safeMarginLeft, Qaterial.Style.card.horizontalPadding)
		   -Math.max(Client.safeMarginRight, Qaterial.Style.card.horizontalPadding)*/
	horizontalAlignment: Qt.AlignHCenter
	font: Qaterial.Style.textTheme.headline6
	icon.width: 1.5 * font.pixelSize
	icon.height: 1.5 * font.pixelSize
	wrapMode: Text.Wrap
}
