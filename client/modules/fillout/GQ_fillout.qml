import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 500 * Qaterial.Style.pixelSizeRatio
	implicitWidth: 650 * Qaterial.Style.pixelSizeRatio

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

					implicitWidth: Qaterial.Style.gameButtonImplicitHeight*1.2

					allowResizeToContent: true
					allowReplaceContent: true

					property string wordId: ""
				}
			}

			Component {
				id: _cmpWord

				Qaterial.Label {
					height: Qaterial.Style.gameButtonImplicitHeight
					verticalAlignment: Text.AlignVCenter

					font.family: questionData.monospace ? "Ubuntu Mono" : Qaterial.Style.textTheme.body1.family
					font.pixelSize: Qaterial.Style.textTheme.body1.pixelSize
					font.weight: Qaterial.Style.textTheme.body1.weight
					font.letterSpacing: Qaterial.Style.textTheme.body1.letterSpacing
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

			fontFamily: questionData.monospace ? "Ubuntu Mono" : ""
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

				if (toggleMode == GameQuestionComponentImpl.ToggleNone)
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
					if (toggleMode == GameQuestionComponentImpl.ToggleNone)
						d.currentDrag.buttonType = GameQuestionButton.Correct
				} else {
					a.success = false
					if (toggleMode == GameQuestionComponentImpl.ToggleNone)
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

	Keys.onPressed: event => {
						var key = event.key

						if (key === Qt.Key_Return || key === Qt.Key_Enter)
						titleRow.buttonOkClicked()
					}
}

