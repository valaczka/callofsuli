import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

QScrollable {
	id: root

	property ConquestGame game: null

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
