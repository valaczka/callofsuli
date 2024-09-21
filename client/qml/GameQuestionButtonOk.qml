import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionButton {
	id: btnOk

	icon.source: Qaterial.Icons.checkBold
	text: qsTr("Kész")

	backgroundColor: enabled ? Qaterial.Colors.green700 : "transparent"
	foregroundColor: enabled ? Qaterial.Colors.white : Qaterial.Colors.cyanA100
	outlinedColor: enabled ? Qaterial.Colors.green300 : Qaterial.Style.dividersColor()
}
