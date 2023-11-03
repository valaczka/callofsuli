import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.RaisedButton {
	id: control

	highlighted: false

	property color textColor: Qaterial.Style.primaryTextColor()
	property color bgColor: Qaterial.Style.buttonColor
	property color highlightedTextColor: Qaterial.Colors.black

	foregroundColor:
	{
		if(!enabled)
			return Qaterial.Style.disabledTextColor()
		if(flat && highlighted)
			return palette.highlight

		if (highlighted)
			return control.highlightedTextColor

		return colorReversed ? Qaterial.Style.primaryTextColorReversed() : control.textColor
	}

	backgroundColor:
	{
		if(flat)
			return (outlined && pressed) ? Qaterial.Style.backgroundColor : "transparent"
		if(!enabled)
			return Qaterial.Style.buttonDisabledColor
		return highlighted ? palette.highlight : control.bgColor
	}
}
