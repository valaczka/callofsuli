import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.TabBar
{
	id: control
	width: parent.width

	property alias model: tabBarModel

	accentColor: Qaterial.Style.iconColor()

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
			icon.color: foregroundColor
			icon.width: (index === control.currentIndex) ? Qaterial.Style.tabButton.iconWidth*1.2 : Qaterial.Style.tabButton.iconWidth*1.8
			icon.height: (index === control.currentIndex) ? Qaterial.Style.tabButton.iconWidth*1.2 : Qaterial.Style.tabButton.iconWidth*1.8
			spacing: 4
			display: (index === control.currentIndex) ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
			font: Qaterial.Style.textTheme.buttonTab
			foregroundColor: model.color ? model.color :
										   (index === control.currentIndex) ? Qaterial.Style.accentColor : Qaterial.Style.iconColor()
		}
	}
}
