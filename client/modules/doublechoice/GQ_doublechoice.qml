import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: titleRow.implicitHeight
					+(Qaterial.Style.gameButtonImplicitHeight*(_listA.model.length + _listB.model.length))
					+(_listA.spacing * _listA.model.length)
					+(_listB.spacing * _listB.model.length)
					+35
	implicitWidth: 700

	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: true
		buttonOkEnabled: _listA.selectedButtonIndex != -1 && _listB.selectedButtonIndex != -1

		title: questionData.question

		onButtonOkClicked: if (_listA.selectedButtonIndex != -1 && _listB.selectedButtonIndex != -1)
							   answer(_listA.selectedButtonIndex, _listB.selectedButtonIndex)

	}


	GridLayout {
		id: _layout

		columns: isHorizontal ? 2 : 1

		readonly property bool isHorizontal: control.width > control.height

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		DoubleChoiceList {
			id: _listA

			Layout.fillWidth: true
			Layout.fillHeight: true

			isHorizontal: _layout.isHorizontal
			toogleMode: control.toggleMode

			showSeparator: true

			model: questionData.optionsA

			Connections {
				target: control

				function onAnswerReveal(answer) {
					_listA.answerReveal(questionData.answer.first, answer.indexA)
				}
			}
		}

		DoubleChoiceList {
			id: _listB

			Layout.fillWidth: true
			Layout.fillHeight: true

			isHorizontal: _layout.isHorizontal
			toogleMode: control.toggleMode

			model: questionData.optionsB

			Connections {
				target: control

				function onAnswerReveal(answer) {
					_listB.answerReveal(questionData.answer.second, answer.indexB)
				}
			}
		}
	}


	onQuestionChanged: {
		if (storedAnswer.indexA !== undefined)
			_listA.selectedButtonIndex = storedAnswer.indexA

		if (storedAnswer.indexB !== undefined)
			_listB.selectedButtonIndex = storedAnswer.indexB
	}


	function answer(idxA, idxB) {
		if (idxA === questionData.answer.first && idxB === questionData.answer.second)
			question.onSuccess({ "indexA": idxA, "indexB": idxB })
		else
			question.onFailed({ "indexA": idxA, "indexB": idxB })
	}



	/*Keys.onPressed: event => {
						var key = event.key

						if ((control.toggleMode == GameQuestionComponentImpl.ToggleSelect ||
							 control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) &&
							selectedButtonIndex != -1 && (key === Qt.Key_Return || key === Qt.Key_Enter)) {
							titleRow.buttonOkClicked()
							return
						}

						var n = -1

						if (key === Qt.Key_1)
						n = 0
						else if (key === Qt.Key_2)
						n = 1
						else if (key === Qt.Key_3)
						n = 2
						else if (key === Qt.Key_4)
						n = 3
						else if (key === Qt.Key_5)
						n = 4
						else if (key === Qt.Key_6)
						n = 5

						if (n < 0)
						return

						if (toggleMode == GameQuestionComponentImpl.ToggleSelect ||
							toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							selectedButtonIndex = n
						}

						if (toggleMode == GameQuestionComponentImpl.ToggleNone ||
							toggleMode == GameQuestionComponentImpl.ToggleFeedback) {
							answer(n)
						}
					}*/
}

