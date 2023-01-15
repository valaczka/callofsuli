import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.RaisedButton {
	id: control

	highlighted: false

	foregroundColor:
	{
	  if(!enabled)
		return Qaterial.Style.disabledTextColor()
	  if(flat && highlighted)
		return palette.highlight

	  if (highlighted)
		  return Qaterial.Colors.black

	  return colorReversed ? Qaterial.Style.primaryTextColorReversed() : Qaterial.Style.primaryTextColor()
	}
}
