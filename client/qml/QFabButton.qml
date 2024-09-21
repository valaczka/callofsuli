import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.FabButton
{
	id: control

	anchors.right: parent.right
	anchors.rightMargin: Qaterial.Style.fab.anchorsOffset
	anchors.bottom: parent.bottom
	anchors.bottomMargin: Qaterial.Style.fab.anchorsOffset

	foregroundColor:
	{
		if(!enabled)
			return flat ? Qaterial.Style.disabledTextColor() : Qaterial.Style.disabledTextColor()
		if(flat && highlighted)
			return Qaterial.Style.buttonAccentColor

		if (highlighted)
			return Qaterial.Colors.black

		return flat ? Qaterial.Style.secondaryTextColor() : Qaterial.Style.buttonTextColor
	}
}
