import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.TabBar
{
	id: control
	width: parent.width

	property alias model: tabBarModel

	accentColor: Qaterial.Style.accentColor

	highlightItem: null

	Repeater
	{
		id: repeater

		model: ListModel
		{
			id: tabBarModel
		}

		delegate: Qaterial.TabButton
		{
			width: control.width / repeater.model.count
			implicitWidth: width
			text: model.text ? model.text : ""
			icon.source: model.source ? model.source : ""
			icon.color: foregroundColor
			icon.width: (index === control.currentIndex) ? Qaterial.Style.tabButton.iconWidth*1.2 : Qaterial.Style.tabButton.iconWidth*1.8
			icon.height: (index === control.currentIndex) ? Qaterial.Style.tabButton.iconWidth*1.2 : Qaterial.Style.tabButton.iconWidth*1.8
			spacing: 4
			display: (index === control.currentIndex) ? AbstractButton.TextUnderIcon : AbstractButton.IconOnly
			font: Qaterial.Style.textTheme.buttonTab
			foregroundColor: (index === control.currentIndex)
							 ? (model.color ? model.color : Qaterial.Style.accentColor)
							 : Qaterial.Style.disabledTextColor()
		}
	}
}
