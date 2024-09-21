import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Item {
	id: root

	enum AnswerState {
		AnswerInvalid,
		AnswerFailed,
		AnswerSuccess
	}

	property string character: ""
	property bool flipped: false
	property int answerState: ConquestBattleInfoPlayer.AnswerInvalid
	property real answerMsec: 0

	width: 75
	height: 75

	Image {
		id: _img1
		fillMode: Image.PreserveAspectFit
		width: 50
		height: 50
		anchors.centerIn: parent

		source: character != "" ? "qrc:/character/%1/thumbnail.png".arg(character) : ""

		transform: Scale {
			xScale: flipped ? -1 : 1
			origin.x: _img1.width/2
			origin.y: _img1.height/2
		}
	}

	Qaterial.LabelCaption {
		id: _msec1
		color: Qaterial.Colors.white
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.top: parent.top
		text: answerMsec > 0 ? Number(answerMsec/1000).toLocaleString(Qt.locale(), "f", 3)+qsTr(" mp") : ""
	}

	Qaterial.Icon {
		id: _icon1
		anchors.right: parent.right
		anchors.bottom: parent.bottom
		anchors.margins: 3

		icon: answerState === ConquestBattleInfoPlayer.AnswerSuccess ? Qaterial.Icons.checkCircle : Qaterial.Icons.closeCircle
		color: answerState === ConquestBattleInfoPlayer.AnswerSuccess ? Qaterial.Colors.green400 : Qaterial.Colors.red400
		size: Qaterial.Style.largeIcon
		opacity: answerState === ConquestBattleInfoPlayer.AnswerInvalid ? 0.0 : 1.0
		visible: opacity

		Behavior on opacity {
			NumberAnimation { duration: 350; easing.type: Easing.OutQuad }
		}
	}
}
