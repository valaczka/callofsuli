import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionButton {
	id: btnPostpone
	icon.source: Qaterial.Icons.skipForward
	text: qsTr("Később")

	backgroundColor: Qaterial.Colors.amber600
	foregroundColor: Qaterial.Colors.black
	outlinedColor: Qaterial.Colors.black
}
