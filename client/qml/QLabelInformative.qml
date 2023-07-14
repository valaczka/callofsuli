import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.LabelBody2 {
	anchors.horizontalCenter: parent.horizontalCenter
	width: Math.min(implicitWidth, parent.width, Qaterial.Style.maxContainerSize, 768*Qaterial.Style.pixelSizeRatio*0.85)
	wrapMode: Text.Wrap
	horizontalAlignment: Text.AlignHCenter
	color: Qaterial.Style.secondaryTextColor()
}
