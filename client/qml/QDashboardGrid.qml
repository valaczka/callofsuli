import QtQuick 2.12
import QtQuick.Controls 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Grid {
	id: control

	property real gridContentWidth: Math.min(parent.width-10, 700)
	property real buttonSize: Qaterial.Style.dashboardButtonSize

	columns: Math.floor(gridContentWidth/(buttonSize+spacing))
	spacing: buttonSize*0.2

	topPadding: spacing
	bottomPadding: spacing
}
