import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.RaisedButton {
	id: control

	highlighted: false

	property color textColor: Qaterial.Style.primaryTextColor()
	property color bgColor: Qaterial.Style.buttonColor
	property color highlightedTextColor: Qaterial.Colors.black
	property color highlightedBgColor: Qaterial.Style.accentColor

	foregroundColor:
	{
		if(!enabled)
			return Qaterial.Style.disabledTextColor()
		if(flat && highlighted)
			return highlightedBgColor

		if (highlighted)
			return highlightedTextColor

		return colorReversed ? Qaterial.Style.primaryTextColorReversed() : textColor
	}

	backgroundColor:
	{
		if(flat)
			return (outlined && pressed) ? Qaterial.Style.backgroundColor : "transparent"
		if(!enabled)
			return Qaterial.Style.buttonDisabledColor
		return highlighted ? highlightedBgColor : bgColor
	}
}
