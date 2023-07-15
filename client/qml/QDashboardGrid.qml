import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Grid {
	id: control

	property real buttonSize: Qaterial.Style.dashboardButtonSize
	property real horizontalPadding: 10
	property real gridContentWidth: Math.min(parent.width-2*horizontalPadding, Qaterial.Style.maxContainerSize)
	property int contentItems: -1

	columns: contentItems > 0 ? Math.min(contentItems, Math.floor(gridContentWidth/(buttonSize+spacing)))
							  : Math.min(visibleChildren.length, Math.floor(gridContentWidth/(buttonSize+spacing)))
	spacing: buttonSize*0.2

	topPadding: spacing
	bottomPadding: spacing
}
