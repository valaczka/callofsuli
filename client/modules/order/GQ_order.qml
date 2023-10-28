import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 450
	implicitWidth: 650

	GameQuestionTitle {
		id: titleRow

		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		buttons: true
		buttonOkEnabled: true

		title: questionData.question

		onButtonOkClicked: answer()
	}

	GameQuestionDNDlayout {
		id: containerItem

		anchors.right: parent.right
		anchors.left: parent.left
		anchors.top: parent.top
		anchors.bottom: titleRow.top
		anchors.topMargin: 15

		flowSplit: 0.5

		contentSourceComponent: Column {
			id: _column
			width: containerItem.implicitContentWidth
			spacing: 3

			Component {
				id: _cmpDrop

				GameQuestionDNDdrop {
					width: parent.width
					allowResizeToContent: false
					allowReplaceContent: true
				}
			}

			Connections {
				target: control
				function onQuestionChanged() {
					if (!control.questionData || !control.questionData.list)
						return

					for (var i=0; i<control.questionData.list.length; i++) {
						var p = control.questionData.list[i]

						var d = _cmpDrop.createObject(_column)

						if (i == 0)
							d.placeholderText = (control.questionData.mode === "descending") ?
										control.questionData.placeholderMax : control.questionData.placeholderMin
						else if (i == control.questionData.list.length-1)
							d.placeholderText = (control.questionData.mode === "descending") ?
										control.questionData.placeholderMin : control.questionData.placeholderMax


						containerItem.drops.push(d)
					}

					loadStoredAnswer()
				}
			}
		}
	}


	Component {
		id: _cmp

		GameQuestionDNDbutton {
			drops: containerItem.drops
			property int num: -1
		}
	}

	onQuestionChanged: {
		if (!questionData || !questionData.list)
			return

		for (var i=0; i<questionData.list.length; i++) {
			var t = questionData.list[i]
			containerItem.createDND(_cmp, control, {
												text: t.text,
												num: t.num,
												dragIndex: i
											})
		}

		loadStoredAnswer()
	}

	function loadStoredAnswer() {
		if (storedAnswer.list !== undefined)
			containerItem.loadFromList(storedAnswer.list)
	}

	function answer() {
		let success = true

		let l = []

		for (var i=0; i<containerItem.drops.length; ++i) {
			var d=containerItem.drops[i]

			let a = {}

			if (!d.currentDrag) {
				success = false

				if (!toggleMode)
					d.showAsError = true

				a.answer = ""
				a.success = false
				a.dragIndex = -1

			} else {
				let t = d.currentDrag.text
				let n = d.currentDrag.num

				let s = (questionData.answer[i] === n)

				a.answer = t
				a.dragIndex = d.currentDrag.dragIndex

				if (s) {
					a.success = true
					if (!toggleMode)
						d.currentDrag.buttonType = GameQuestionButton.Correct
				} else {
					a.success = false
					if (!toggleMode)
						d.currentDrag.buttonType = GameQuestionButton.Wrong
					success = false
				}
			}

			l.push(a)
		}

		if (success)
			question.onSuccess({"list": l})
		else
			question.onFailed({"list": l})

	}


	Keys.onPressed: {
		var key = event.key

		if (key === Qt.Key_Return || key === Qt.Key_Enter)
			titleRow.buttonOkClicked()
	}
}

