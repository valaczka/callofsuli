import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: titleRow.implicitHeight+row.implicitHeight+150
	implicitWidth: 700 * Qaterial.Style.pixelSizeRatio

	property int selectedButtonIndex: -1

	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: control.toggleMode == GameQuestionComponentImpl.ToggleSelect
		buttonOkEnabled: control.toggleMode == GameQuestionComponentImpl.ToggleSelect && selectedButtonIndex != -1

		title: questionData.question

		onButtonOkClicked: if (selectedButtonIndex != -1)
							   answer(selectedButtonIndex)

	}

	Item {
		id: containerItem

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		readonly property real buttonMinWidth: 150 * Qaterial.Style.pixelSizeRatio

		Row {
			id: row
			anchors.centerIn: parent
			spacing: 30


			Repeater {
				model: [qsTr("Hamis"), qsTr("Igaz")]

				delegate: GameQuestionButton {
					id: btn
					text: modelData
					width: Math.max(implicitWidth, containerItem.buttonMinWidth)

					buttonType: control.toggleMode == GameQuestionComponentImpl.ToggleSelect ||
								control.toggleMode == GameQuestionComponentImpl.ToggleFeedback ?
									(control.selectedButtonIndex === index ? GameQuestionButton.Selected : GameQuestionButton.Neutral) :
									GameQuestionButton.Neutral

					onClicked: {
						if (control.toggleMode == GameQuestionComponentImpl.ToggleSelect ||
								control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							control.selectedButtonIndex = index
						}

						if (control.toggleMode == GameQuestionComponentImpl.ToggleNone ||
								control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							answer(index)
						}
					}

					Connections {
						target: control
						function onAnswerReveal(answer) {
							if (index === questionData.answer)
								btn.buttonType = GameQuestionButton.Correct
							else if (answer.index === index || selectedButtonIndex === index)
								btn.buttonType = GameQuestionButton.Wrong
						}
					}
				}
			}


		}
	}


	onQuestionChanged: {
		if (storedAnswer.index !== undefined) {
			selectedButtonIndex = storedAnswer.index
		}
	}


	function answer(idx) {
		if (idx === questionData.answer)
			question.onSuccess({"index": idx})
		else
			question.onFailed({"index": idx})
	}


	Keys.onPressed: event => {
						var key = event.key

						if (control.toggleMode == GameQuestionComponentImpl.ToggleSelect ||
							control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							if (key === Qt.Key_I || key === Qt.Key_Y)
							selectedButtonIndex = 1
							else if (key === Qt.Key_N || key === Qt.Key_H)
							selectedButtonIndex = 0
						}

						if (control.toggleMode == GameQuestionComponentImpl.ToggleNone ||
							control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							if (key === Qt.Key_I || key === Qt.Key_Y)
							answer(1)
							else if (key === Qt.Key_N || key === Qt.Key_H)
							answer(0)
						}
					}


}

