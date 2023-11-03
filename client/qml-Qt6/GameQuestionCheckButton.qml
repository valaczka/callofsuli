import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
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
	horizontalAlignment: Qt.AlignLeft

	onCheckedChanged: buttonType = (checked ? GameQuestionButton.Selected : GameQuestionButton.Neutral )
}
