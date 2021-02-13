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
	signal buttonPressByKey(int num)

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
		leftPadding: 20
		rightPadding: 20

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		textFormat: Text.RichText

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

						function onButtonPressByKey(num) {
							if (num-1 == index) {
								btn.clicked()
							}
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

	function keyPressed(key) {
		if (key === Qt.Key_1 || key === Qt.Key_A)
			buttonPressByKey(1)
		else if (key === Qt.Key_2 || key === Qt.Key_B)
			buttonPressByKey(2)
		else if (key === Qt.Key_3 || key === Qt.Key_C)
			buttonPressByKey(3)
		else if (key === Qt.Key_4 || key === Qt.Key_D)
			buttonPressByKey(4)
		else if (key === Qt.Key_5 || key === Qt.Key_E)
			buttonPressByKey(5)
		else if (key === Qt.Key_6 || key === Qt.Key_F)
			buttonPressByKey(6)
	}
}

