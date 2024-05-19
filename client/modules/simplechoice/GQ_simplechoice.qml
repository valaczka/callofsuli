import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: imageButtons || img.visible ? 500
												: titleRow.implicitHeight
												  +(Qaterial.Style.gameButtonImplicitHeight*rptr.model.length)
												  +(grid.rowSpacing*rptr.model.length)
												  +35
	implicitWidth: 700

	readonly property bool imageButtons: questionData.imageAnswers === true

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

		readonly property bool isHorizontal: control.width > control.height

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		Image {
			id: img
			source: questionData.image !== undefined ? questionData.image : ""
			visible: questionData.image !== undefined

			width: (containerItem.isHorizontal ? parent.width/2 : parent.width)
			height: (containerItem.isHorizontal ? parent.height : parent.height/2)-10

			anchors.top: parent.top
			anchors.left: parent.left
			anchors.topMargin: 5
			anchors.leftMargin: 5

			fillMode: Image.PreserveAspectFit
			cache: true
			//asynchronous: true

			sourceSize.width: width
			sourceSize.height: height
		}

		Flickable {
			id: flick

			anchors.top: img.visible && !containerItem.isHorizontal ? img.bottom : parent.top
			anchors.left: img.visible && containerItem.isHorizontal ? img.right: parent.left
			anchors.right: parent.right
			anchors.bottom: parent.bottom
			anchors.topMargin: img.visible && !containerItem.isHorizontal ? 10 : 0
			anchors.leftMargin: img.visible && containerItem.isHorizontal ? 10 : 20
			anchors.rightMargin: img.visible && containerItem.isHorizontal ? 10 : 20

			clip: true

			contentWidth: grid.width
			contentHeight: grid.height

			boundsBehavior: Flickable.StopAtBounds
			flickableDirection: Flickable.VerticalFlick

			//ScrollIndicator.vertical: ScrollIndicator { active: flick.movingVertically || flick.contentHeight > flick.height }
			ScrollBar.vertical: ScrollBar {
				id: _scrollBar
				parent: containerItem
				anchors.top: flick.top
				anchors.right: flick.right
				anchors.bottom: flick.bottom
				policy: flick.contentHeight > flick.height ? ScrollBar.AlwaysOn : ScrollBar.AlwaysOff
			}



			Grid {
				id: grid

				width: flick.width - (_scrollBar.visible ? _scrollBar.width : 0)

				y: Math.max((flick.height-height)/2, 0)
				rowSpacing: 3
				columnSpacing: 3

				columns: imageButtons ? 2 : 1

				Repeater {
					id: rptr
					model: questionData.options
					delegate: imageButtons ? cmpImage : cmpNormal
				}
			}
		}
	}


	Component {
		id: cmpNormal

		GameQuestionButton {
			id: btn
			text: modelData
			width: grid.width

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



	Component {
		id: cmpImage

		GameQuestionButton {
			id: btn
			//text: "kérdés"
			width: (grid.width-grid.columnSpacing)/2
			height: (flick.height-grid.rowSpacing)/2

			icon.source: modelData
			icon.color: "transparent"
			icon.width: btn.width-10
			icon.height: btn.height-10

			display: AbstractButton.IconOnly

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

						if ((control.toggleMode == GameQuestionComponentImpl.ToggleSelect ||
							 control.toggleMode == GameQuestionComponentImpl.ToggleFeedback) &&
							selectedButtonIndex != -1 && (key === Qt.Key_Return || key === Qt.Key_Enter)) {
							titleRow.buttonOkClicked()
							return
						}

						var n = -1

						if (key === Qt.Key_1 || key === Qt.Key_A)
						n = 0
						else if (key === Qt.Key_2 || key === Qt.Key_B)
						n = 1
						else if (key === Qt.Key_3 || key === Qt.Key_C)
						n = 2
						else if (key === Qt.Key_4 || key === Qt.Key_D)
						n = 3
						else if (key === Qt.Key_5 || key === Qt.Key_E)
						n = 4
						else if (key === Qt.Key_6 || key === Qt.Key_F)
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
					}
}

