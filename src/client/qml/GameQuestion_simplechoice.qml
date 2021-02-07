import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.implicitHeight+col.implicitHeight+25
	implicitWidth: 700

	property var questionData: null

	property real buttonWidth: width-60

	signal succeed()
	signal failed()

	signal buttonReveal(GameQuestionButton original)

	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.top: parent.top
		anchors.left: parent.left
		anchors.right: parent.right
		topPadding: 50
		bottomPadding: 50
		leftPadding: 10
		rightPadding: 10

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		text: questionData.question
	}

	Item {
		anchors.top: labelQuestion.bottom
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.bottom: parent.bottom
		anchors.bottomMargin: 20

		Column {
			id: col

			anchors.centerIn: parent
			spacing: 5

			Repeater {
				id: rptr
				model: questionData.options

				GameQuestionButton {
					id: btn
					text: modelData.answer
					width: buttonWidth
					onClicked: {
						answer(modelData.correct, btn)
						type = modelData.correct ? GameQuestionButton.Correct : GameQuestionButton.Wrong
					}

					Connections {
						target: control
						function onButtonReveal(original) {
							btn.interactive = false

							if (original === btn)
								return

							if (modelData.correct)
								btn.type = GameQuestionButton.Correct
						}
					}
				}
			}
		}
	}


	function answer(correct, btn) {
		buttonReveal(btn)

		if (correct)
			succeed()
		else
			failed()
	}

	function clickBtn(t) {

	}
}

