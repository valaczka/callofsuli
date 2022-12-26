import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: titleRow.implicitHeight+row.implicitHeight+150
	implicitWidth: 700

	property int selectedButtonIndex: -1

	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: control.toggleMode
		buttonOkEnabled: control.toggleMode && selectedButtonIndex != -1

		title: questionData.question

		onButtonOkClicked: answer(selectedButtonIndex)

	}

	Item {
		id: containerItem

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		Row {
			id: row
			anchors.centerIn: parent
			spacing: 30

			property real buttonMinWidth: 150 * Qaterial.Style.pixelSizeRatio

			Repeater {
				model: [qsTr("Hamis"), qsTr("Igaz")]

				delegate: GameQuestionButton {
					id: btn
					text: modelData
					width: Math.max(implicitWidth, row.buttonMinWidth)

					buttonType: control.toggleMode ?
									(control.selectedButtonIndex === index ? GameQuestionButton.Selected : GameQuestionButton.Neutral) :
									GameQuestionButton.Neutral

					onClicked: {
						if (control.toggleMode) {
							control.selectedButtonIndex = index
						} else {
							answer(index)
						}
					}

					Connections {
						target: control
						function onAnswerReveal(answer) {
							if (index === questionData.answer)
								btn.buttonType = GameQuestionButton.Correct
							else if (answer.value === index || selectedButtonIndex === index)
								btn.buttonType = GameQuestionButton.Wrong
						}
					}
				}
			}


		}
	}



	function answer(idx) {
		if (idx === questionData.answer)
			question.onSuccess({"value": idx})
		else
			question.onFailed({"value": idx})
	}


	Keys.onPressed: {
		var key = event.key

		if (key === Qt.Key_Return || key === Qt.Key_Enter || key === Qt.Key_I || key === Qt.Key_Y)
			answer(1)
		else if (key === Qt.Key_N || key === Qt.Key_H)
			answer(0)
	}


}

