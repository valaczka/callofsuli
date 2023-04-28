import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 550
	implicitWidth: 800

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

		contentSourceComponent: GridLayout {
			id: _grid
			width: containerItem.implicitContentWidth
			columns: 3
			columnSpacing: 15
			rowSpacing: 3

			Component {
				id: _cmpLabel

				Qaterial.LabelBody1 {
					Layout.fillWidth: false
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter
					Layout.maximumWidth: containerItem.implicitContentWidth*0.5
					wrapMode: Text.Wrap
				}
			}

			Component {
				id: _cmpSep

				Qaterial.LabelSubtitle1 {
					Layout.fillWidth: false
					Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
					text: "—"
				}
			}

			Component {
				id: _cmpDrop

				GameQuestionDNDdrop {
					allowResizeToContent: false
					allowReplaceContent: true
					Layout.fillWidth: true
					Layout.maximumWidth: containerItem.implicitContentWidth*0.5
					Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

					width: implicitWidth

					property string correctAnswer: ""
				}
			}

			Connections {
				target: control
				function onQuestionDataChanged() {
					if (!control.questionData || !control.questionData.list)
						return

					for (var i=0; i<control.questionData.list.length; i++) {
						var p = control.questionData.list[i]

						_cmpLabel.createObject(_grid, { text: p })
						_cmpSep.createObject(_grid)

						var d = _cmpDrop.createObject(_grid)

						containerItem.drops.push(d)

						if (control.questionData.answer && control.questionData.answer.list && control.questionData.answer.list.length>i)
							d.correctAnswer = questionData.answer.list[i]
					}
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


	onQuestionDataChanged: {
		if (!questionData || !questionData.options)
			return

		for (var i=0; i<questionData.options.length; i++) {
			var t = questionData.options[i]
			containerItem.dndFlow.createDND(_cmp, control, { text: t })
		}
	}

	function answer() {
		var success = true

		var l = []

		for (var i=0; i<containerItem.drops.length; ++i) {
			var d=containerItem.drops[i]

			var a = {}

			if (!d.currentDrag) {
				success = false
				d.showAsError = true
				a.answer = ""
				a.success = false
			} else {
				var t = d.currentDrag.text
				a.answer = t
				if (t === d.correctAnswer) {
					a.success = true
					d.currentDrag.buttonType = GameQuestionButton.Correct
				} else {
					a.success = false
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

}

