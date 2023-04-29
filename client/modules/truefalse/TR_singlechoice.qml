import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial

GameTestResultBase {
	id: root

	Row {
		spacing: 10

		GameTestResultLabel {
			anchors.verticalCenter: parent.verticalCenter
			text: qsTr("Igaz vagy hamis?")
		}

		GameTestResultLabelAnswer {
			anchors.verticalCenter: parent.verticalCenter
			success: _success
			text: _answer.index !== undefined ?
					  _answer.index ? qsTr("IGAZ") : qsTr("HAMIS")
			: ""
		}

		GameTestResultCheckmark {
			anchors.verticalCenter: parent.verticalCenter
			visible: _success
		}
	}

}
