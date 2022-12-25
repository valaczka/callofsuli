import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 300 //labelQuestion.implicitHeight+row.implicitHeight+25
	implicitWidth: 800

	Row {
		anchors.centerIn: parent

		GameQuestionButton {
			id: btn1
			text: "skdjald fasdoiw erélkadf laksjdfarwe"
			onClicked: {
				buttonType++
				if (buttonType > GameQuestionButton.Selected)
					buttonType = GameQuestionButton.Neutral
			}
		}

		GameQuestionButton {
			id: btn2
			text: "skarwe"
			onClicked: btn1.enabled = !btn1.enabled
		}

		GameQuestionButton {
			text: "Accept"
			onClicked: question.onSuccess({})
		}

		GameQuestionButton {
			text: "Failed"
			onClicked: question.onFailed({})
		}

		GameQuestionCheckButton {
			text: "Check asfjaowe rlkafa"
		}
	}

	Keys.onPressed: console.debug("PRESS", event)


	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: false

		title: "iasdoai sdfélkasd joiauw sdf oaiu faklésdjf awioeru alkdsfj aéirué lafj alouoirerlakdf?"
	}

	/*QButton {
		id: btnPostpone
		enabled: canPostpone
		visible: canPostpone
		anchors.verticalCenter: labelQuestion.verticalCenter
		anchors.left: parent.left
		anchors.leftMargin: 20
		icon.source: CosStyle.iconPostpone
		text: qsTr("Később")
		themeColors: CosStyle.buttonThemeOrange
		onClicked: postponed()
	}

	QLabel {
		id: labelQuestion

		font.family: "Special Elite"
		font.pixelSize: CosStyle.pixelSize*1.4
		wrapMode: Text.Wrap
		anchors.bottom: parent.bottom
		anchors.left: btnPostpone.visible ? btnPostpone.right : parent.left
		anchors.right: btnOk.visible ? btnOk.left : parent.right
		height: Math.max(implicitHeight, btnOk.height)
		topPadding: 25
		bottomPadding: 25
		leftPadding: 20
		rightPadding: 20

		horizontalAlignment: Text.AlignHCenter

		color: CosStyle.colorAccent

		textFormat: Text.RichText

		text: questionData.question
	}


	QButton {
		id: btnOk
		enabled: (control.mode == GameMatch.ModeExam)
		visible: (control.mode == GameMatch.ModeExam)
		anchors.verticalCenter: labelQuestion.verticalCenter
		anchors.right: parent.right
		anchors.rightMargin: 20
		icon.source: "qrc:/internal/icon/check-bold.svg"
		text: qsTr("Kész")
		themeColors: CosStyle.buttonThemeGreen
		onClicked: answer()
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

			property real buttonWidth: Math.min(Math.max(btnTrue.implicitWidth, btnFalse.implicitWidth, 120), control.width/2-40)

			GameQuestionButton {
				id: btnTrue
				text: qsTr("Igaz")
				onClicked: { answer(true) }
				width: row.buttonWidth

				onToggled: btnFalse.selected = false
				isToggle: (control.mode == GameMatch.ModeExam)
			}

			GameQuestionButton {
				id: btnFalse
				text: qsTr("Hamis")
				onClicked: { answer(false) }
				width: row.buttonWidth

				onToggled: btnTrue.selected = false
				isToggle: (control.mode == GameMatch.ModeExam)
			}
		}
	}


	function answer(correct) {
		btnTrue.interactive = false
		btnFalse.interactive = false

		if (mode == GameMatch.ModeExam) {
			var i = -1
			if (btnFalse.selected)
				i = 0
			else if (btnTrue.selected)
				i = 1

			answered({value: i})
		} else {
			if (correct === questionData.answer)
				succeed()
			else
				failed()

			if (questionData.answer) {
				btnTrue.type = GameQuestionButton.Correct
				if (!correct) btnFalse.type = GameQuestionButton.Wrong
			} else {
				btnFalse.type = GameQuestionButton.Correct
				if (!correct) btnTrue.type = GameQuestionButton.Wrong
			}
		}
	}


	function keyPressed(key) {
		if (btnTrue.interactive && (key === Qt.Key_Enter || key === Qt.Key_I || key === Qt.Key_Y)) {
			if (btnTrue.isToggle) {
				btnTrue.selected = true
				btnTrue.toggled()
			} else {
				btnTrue.clicked()
			}
		} else if (btnFalse.interactive && (key === Qt.Key_N || key === Qt.Key_H)) {
			if (btnFalse.isToggle) {
				btnFalse.selected = true
				btnFalse.toggled()
			} else {
				btnFalse.clicked()
			}
		}
	}*/
}

