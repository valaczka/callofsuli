import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.RaisedButton {
	id: control

	highlighted: false

	property color textColor: Qaterial.Style.primaryTextColor()
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
}
