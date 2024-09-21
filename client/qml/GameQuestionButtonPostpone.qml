import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionButton {
	id: btnPostpone
	icon.source: Qaterial.Icons.skipForward
	text: qsTr("Később")

	backgroundColor: Qaterial.Colors.amber600
	foregroundColor: Qaterial.Colors.black
	outlinedColor: Qaterial.Colors.black
}
