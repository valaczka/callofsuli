import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 500
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

		contentSourceComponent: Flow {
			id: _flow
			width: containerItem.implicitContentWidth
			spacing: 5

			move: Transition {
				NumberAnimation { properties: "x,y"; duration: 250; easing.type: Easing.OutQuad }
			}

			Component {
				id: _cmpDrop

				GameQuestionDNDdrop {
					allowResizeToContent: true
					allowReplaceContent: true

					property string wordId: ""
				}
			}

			Component {
				id: _cmpWord

				Qaterial.LabelBody1 {
					height: Qaterial.Style.gameButtonImplicitHeight
					verticalAlignment: Text.AlignVCenter
				}
			}

			Connections {
				target: control
				function onQuestionChanged() {
					if (!control.questionData || !control.questionData.list)
						return

					for (var i=0; i<control.questionData.list.length; i++) {
						var p = control.questionData.list[i]

						if (p.w !== undefined)
							_cmpWord.createObject(_flow, {text: p.w})
						else if (p.q !== undefined) {
							var d = _cmpDrop.createObject(_flow)
							d.wordId = p.q

							containerItem.drops.push(d)
						}
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
		}
	}

	onQuestionChanged: {
		if (!questionData || !questionData.options)
			return

		for (var i=0; i<questionData.options.length; i++) {
			var t = questionData.options[i]
			containerItem.createDND(_cmp, control, {
										text: t,
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
		var success = true

		var l = []

		for (var i=0; i<containerItem.drops.length; ++i) {
			var d=containerItem.drops[i]

			var a = {}

			if (!d.currentDrag) {
				success = false

				if (!toggleMode)
					d.showAsError = true

				a.answer = ""
				a.dragIndex = -1
				a.success = false
			} else {
				var t = d.currentDrag.text

				var c = questionData.answer ? questionData.answer[d.wordId] : ""

				a.answer = t
				a.dragIndex = d.currentDrag.dragIndex

				if (t === c) {
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

