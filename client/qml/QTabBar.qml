import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.TabBar
{
	id: control
	width: parent.width

	property alias model: tabBarModel

	Repeater
	{
		id: repeater

		model: ListModel
		{
			id: tabBarModel
		}

		delegate: Qaterial.TabButton
		{
			width: control.width / model.count
			implicitWidth: width
			text: model.text ? model.text : ""
			icon.source: model.source ? model.source : ""
			icon.color: model.color ? model.color : foregroundColor
			spacing: 4
			display: (index === control.currentIndex) ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
			font: Qaterial.Style.textTheme.overline
		}
	}
}
