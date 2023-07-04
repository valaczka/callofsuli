import QtQuick 2.15
import QtQuick.Controls 2.15
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionComponentImpl {
	id: control

	implicitHeight: 300
	implicitWidth: 400

	GameQuestionButton {
		anchors.centerIn: parent

		buttonType: GameQuestionButton.Correct

		icon.source: Qaterial.Icons.send
		text: qsTr("Teszt beküldése")

		onClicked: answer()
	}


	function answer() {
		question.onSuccess({"finish": true})
	}


	Keys.onPressed: {
		var key = event.key

		if (key === Qt.Key_Return || key === Qt.Key_Enter) {
			answer()
		}
	}
}

