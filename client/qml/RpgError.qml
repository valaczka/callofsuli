import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QScrollable {
	id: root

	//property ConquestGame game: null

	contentCentered: true

	Qaterial.IconLabel {
		anchors.horizontalCenter: parent.horizontalCenter
		color: Qaterial.Colors.red400
		icon.source: Qaterial.Icons.alertCircle
		icon.width: Qaterial.Style.dashboardButtonSize*0.4
		icon.height: Qaterial.Style.dashboardButtonSize*0.4
		text: qsTr("Hiba!")
	}

}
