import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

Qaterial.Button {
	id: control

	enum ButtonType {
		Neutral,
		Correct,
		Wrong,
		Selected
	}

	property int buttonType: GameQuestionButton.Neutral

	highlighted: false
	flat: true
	outlined: true
	wrapMode: Text.Wrap

	font: Qaterial.Style.textTheme.body2
	backgroundImplicitHeight: 50 //Qaterial.Style.dense ? 48 : 40
	leftPadding: 10
	rightPadding: 10

	opacity: enabled ? 1.0 : 0.7

	foregroundColor: switch (buttonType) {
					 case GameQuestionButton.Correct:
					 case GameQuestionButton.Wrong:
					 case GameQuestionButton.Selected:
						 Qaterial.Colors.white;
						 break
					 case GameQuestionButton.Neutral:
					 default:
						 Qaterial.Colors.cyanA100
						 break
					 }


	backgroundColor: switch (buttonType) {
					 case GameQuestionButton.Correct:
						 Client.Utils.colorSetAlpha(Qaterial.Colors.green700, 0.4);
						 break
					 case GameQuestionButton.Wrong:
						 Client.Utils.colorSetAlpha(Qaterial.Colors.red800, 0.4);
						 break
					 case GameQuestionButton.Selected:
						 Client.Utils.colorSetAlpha(Qaterial.Colors.amber800, 0.4);
						 break
					 case GameQuestionButton.Neutral:
					 default:
						 "transparent"
						 break
					 }

	outlinedColor: switch (buttonType) {
				   case GameQuestionButton.Correct:
					   Qaterial.Colors.green300
					   break
				   case GameQuestionButton.Wrong:
					   Qaterial.Colors.red500;
					   break
				   case GameQuestionButton.Selected:
					   Qaterial.Colors.amber500;
					   break
				   case GameQuestionButton.Neutral:
				   default:
					   Qaterial.Style.dividersColor()
					   break
				   }
}
