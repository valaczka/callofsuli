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

	required property var questionData
	required property var answerData

	signal succeed()
	signal failed()


	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		topPadding: 30
		bottomPadding: 30
		leftPadding: 20
		rightPadding: 20

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		textFormat: Text.RichText

		text: questionData.question
	}

	Item {
		anchors.bottom: labelQuestion.top
		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.topMargin: 20

		Row {
			id: row
			anchors.centerIn: parent
			spacing: 30

			property real buttonWidth: Math.min(Math.max(btnTrue.label.implicitWidth, btnFalse.label.implicitWidth, 120), control.width/2-40)

			GameQuestionButton {
				id: btnTrue
				text: qsTr("Igaz")
				onClicked: { answer(true) }
				width: row.buttonWidth
			}

			GameQuestionButton {
				id: btnFalse
				text: qsTr("Hamis")
				onClicked: { answer(false) }
				width: row.buttonWidth
			}
		}
	}


	function answer(correct) {
		btnTrue.interactive = false
		btnFalse.interactive = false

		if (correct === answerData.correct)
			succeed()
		else
			failed()

		if (answerData.correct) {
			btnTrue.type = GameQuestionButton.Correct
			if (!correct) btnFalse.type = GameQuestionButton.Wrong
		} else {
			btnFalse.type = GameQuestionButton.Correct
			if (!correct) btnTrue.type = GameQuestionButton.Wrong
		}
	}


	function keyPressed(key) {
		if (btnTrue.interactive && (key === Qt.Key_Enter || key === Qt.Key_I || key === Qt.Key_Y))
			btnTrue.clicked()
		else if (btnFalse.interactive && (key === Qt.Key_N || key === Qt.Key_H))
			btnFalse.clicked()
	}
}

