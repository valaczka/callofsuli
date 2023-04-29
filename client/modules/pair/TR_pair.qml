import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial

GameTestResultBase {
	id: root

	GameTestResultLabelOptions {
		width: parent.width
		wrapMode: Text.Wrap
		bottomPadding: 5
		text: _question.options ? _question.options.join(" | ") : ""
	}

	Repeater {
		model: _question.list

		delegate: Row {
			spacing: 10

			readonly property real _width: (root.width - (2*parent.spacing + _sep.width + _checkmark.implicitWidth))/2

			GameTestResultLabel {
				width: _width
				anchors.verticalCenter: parent.verticalCenter
				text: modelData
				wrapMode: Text.Wrap
			}

			GameTestResultLabel {
				id: _sep
				anchors.verticalCenter: parent.verticalCenter
				text: qsTr("--")
			}

			GameTestResultLabelAnswer {
				id: _labelAnswer
				width: Math.min(implicitWidth, _width)
				wrapMode: Text.Wrap
				anchors.verticalCenter: parent.verticalCenter
			}

			GameTestResultCheckmark {
				id: _checkmark
				anchors.verticalCenter: parent.verticalCenter
				visible: false
			}

			Component.onCompleted: {
				if (_answer.list !== undefined && _answer.list.length > index) {
					_labelAnswer.text = _answer.list[index].answer
					_labelAnswer.success = _answer.list[index].success
					_checkmark.visible = _answer.list[index].success
				}
			}
		}
	}


}
