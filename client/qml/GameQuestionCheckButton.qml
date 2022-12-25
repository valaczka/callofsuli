import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionButton {
	id: control

	//icon.width:

	checkable: true
	icon.color: outlinedColor
	icon.height: backgroundImplicitHeight*0.5
	icon.width: backgroundImplicitHeight*0.5
	spacing: leftPadding
	icon.source: checked ? Qaterial.Icons.checkCircle : Qaterial.Icons.checkboxBlankCircleOutline

	onToggled: buttonType = (checked ? GameQuestionButton.Selected : GameQuestionButton.Neutral )
}
