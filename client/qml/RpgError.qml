import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as J

QScrollable {
	id: root

	property ActionRpgMultiplayerGame game: null

	contentCentered: true

	Qaterial.IconLabelWithCaption {
		anchors.horizontalCenter: parent.horizontalCenter
		textColor: Qaterial.Colors.red400
		icon.source: Qaterial.Icons.alertCircle
		icon.width: Qaterial.Style.dashboardButtonSize*0.4
		icon.height: Qaterial.Style.dashboardButtonSize*0.4
		icon.color: textColor
		captionColor: Qaterial.Style.colorTheme.secondaryText
		text: qsTr("Hiba történt")
		caption: game ? game.errorString : ""
	}

}
