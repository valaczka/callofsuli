import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial

GameTestResultBase {
	id: root

	property int answerIdx: -1

	GameTestResultLabelOptions {
		width: parent.width
		wrapMode: Text.Wrap
		bottomPadding: 5
		text: _question.options ? _question.options.join(" | ") : ""
	}

	Row {
		width: parent.width
		spacing: 10

		Flow {
			width: parent.width-parent.spacing-_checkmark.implicitWidth
			anchors.bottom: parent.bottom
			spacing: 5

			Repeater {

				model: _question.list

				delegate: Loader {
					readonly property string word: modelData.w !== undefined ? modelData.w : ""

					Component.onCompleted: {
						if (modelData.w !== undefined)
							sourceComponent = _cmpWord
						else {
							root.answerIdx++
							sourceComponent = _cmpAnswer
						}
					}
				}
			}

		}

		Component {
			id: _cmpWord
			GameTestResultLabel {
				text: word
			}
		}

		Component {
			id: _cmpAnswer
			GameTestResultLabelAnswer {
				topPadding: 0.2*font.pixelSize
				font.underline: true
				Component.onCompleted: {
					var a = ""
					if (_answer.list !== undefined && _answer.list.length > root.answerIdx) {
						a = _answer.list[root.answerIdx].answer
						success = _answer.list[root.answerIdx].success
					}

					text = (a === "" ? "                 " : a)
				}
			}
		}

		GameTestResultCheckmark {
			id: _checkmark
			anchors.bottom: parent.bottom
			visible: _success
		}


	}
}
