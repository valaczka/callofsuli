import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

Qaterial.Card {
	id: root

	property bool rippleActive: hovered
	property bool ripplePressed: pressed

	background: Qaterial.CardBackground
	{
	  isActive: root.isActive
	  onPrimary: root.onPrimary
	  enabled: root.enabled
	  radius: root.radius
	  color: root.backgroundColor
	  borderColor: root.borderColor
	  outlined: root.outlined
	  elevation: root.elevation

	  Qaterial.ListDelegateBackground
	  {
		  anchors.fill: parent
		  type: Qaterial.Style.DelegateType.Icon
		  lines: 1
		  pressed: root.ripplePressed
		  rippleActive: root.rippleActive
		  rippleAnchor: root
	  }
	}
}
