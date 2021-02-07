import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitHeight: labelQuestion.implicitHeight+row.implicitHeight+25
	implicitWidth: 800

	property var questionData: null

	signal succeed()
	signal failed()


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

		Row {
			id: row
			anchors.centerIn: parent
			spacing: 30

			property real buttonWidth: Math.min(Math.max(btnTrue.label.implicitWidth, btnFalse.label.implicitWidth, 120), control.width/2-40)

			GameQuestionButton {
				id: btnTrue
				text: qsTr("Igaz")
				onClicked: { answer(questionData.correct) }
				width: row.buttonWidth
			}

			GameQuestionButton {
				id: btnFalse
				text: qsTr("Hamis")
				onClicked: { answer(!questionData.correct) }
				width: row.buttonWidth
			}
		}
	}


	function answer(correct) {
		btnTrue.interactive = false
		btnFalse.interactive = false

		if (correct)
			succeed()
		else
			failed()

		if (questionData.correct) {
			btnTrue.type = GameQuestionButton.Correct
			if (!correct) btnFalse.type = GameQuestionButton.Wrong
		} else {
			btnFalse.type = GameQuestionButton.Correct
			if (!correct) btnTrue.type = GameQuestionButton.Wrong
		}
	}


	function clickBtn(t) {
		if (key === Qt.Key_Enter || key === Qt.Key_Return)
			btnTrue.clicked()
	}
}

