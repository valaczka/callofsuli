import QtQuick
import QtQuick.Controls
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

Rectangle {
	id: root

	property ConquestGame game: null

	property alias itemFighter1: _fighter1
	property alias itemFighter2: _fighter2

	readonly property int _answerState: game ? game.currentTurn.answerState : ConquestTurn.AnswerPending

	visible: game && game.fighter1.playerId !== -1 && game.fighter2.playerId !== -1
	radius: 5

	implicitHeight: _row.implicitHeight + 50 * Qaterial.Style.pixelSizeRatio
	implicitWidth: _row.implicitWidth + 50 * Qaterial.Style.pixelSizeRatio

	color: Client.Utils.colorSetAlpha(Qaterial.Style.dialogColor, 0.5)

	Row {
		id: _row
		anchors.centerIn: parent
		spacing: 25

		Item {
			id: _fighter1
			width: 75
			height: 75
			//color: "white"

			Image {
				id: _img1
				fillMode: Image.PreserveAspectFit
				width: 50
				height: 50
				anchors.centerIn: parent

				source: game && game.fighter1.playerId !== -1 ? "qrc:/character/%1/thumbnail.png".arg(game.fighter1.character) : ""

				transform: Scale {
					xScale: -1
					origin.x: _img1.width/2
					origin.y: _img1.height/2
				}
			}

			Qaterial.LabelBody2 {
				id: _msec1
				color: Qaterial.Colors.white
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.top: parent.top
			}

			Qaterial.Icon {
				id: _icon1
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 3

				property bool success: false

				icon: success ? Qaterial.Icons.checkCircle : Qaterial.Icons.closeCircle
				color: success ? Qaterial.Colors.green400 : Qaterial.Colors.red400
				size: Qaterial.Style.largeIcon
				opacity: 0.0
				visible: opacity

				Behavior on opacity {
					NumberAnimation { duration: 350; easing.type: Easing.OutQuad }
				}
			}
		}

		Item {
			id: _fighter2
			width: 75
			height: 75
			//color: "white"

			Image {
				id: _img2
				fillMode: Image.PreserveAspectFit
				width: 50
				height: 50
				anchors.centerIn: parent

				source: game && game.fighter2.playerId !== -1 ? "qrc:/character/%1/thumbnail.png".arg(game.fighter2.character) : ""
			}

			Qaterial.LabelBody2 {
				id: _msec2
				color: Qaterial.Colors.white
				anchors.horizontalCenter: parent.horizontalCenter
				anchors.top: parent.top
			}

			Qaterial.Icon {
				id: _icon2
				anchors.right: parent.right
				anchors.bottom: parent.bottom
				anchors.margins: 3

				property bool success: false

				icon: success ? Qaterial.Icons.checkCircle : Qaterial.Icons.closeCircle
				color: success ? Qaterial.Colors.green400 : Qaterial.Colors.red400
				size: Qaterial.Style.largeIcon
				opacity: 0.0
				visible: opacity

				Behavior on opacity {
					NumberAnimation { duration: 350; easing.type: Easing.OutQuad }
				}
			}
		}

		states: [
			State {
				name: "f1"
				when: _answerState == ConquestTurn.AnswerPlayerWin

				PropertyChanges {
					target: _fighter2
					opacity: 0.5
				}
			},

			State {
				name: "f2"
				when: _answerState == ConquestTurn.AnswerPlayerLost

				PropertyChanges {
					target: _fighter1
					opacity: 0.5
				}
			}
		]

		transitions: [
			Transition {
				PropertyAnimation {
					property: "opacity"
					duration: 350
					easing.type: Easing.InOutQuad
				}
			}
		]
	}

	Connections {
		target: game

		function onCurrentTurnChanged() {
			let ms1 = 0
			let ms2 = 0
			let s1 = false
			let s2 = false

			if (game.fighter1.playerId !== -1) {
				ms1 = game.currentTurn.getElapsed(game.fighter1.playerId)
				s1 = game.currentTurn.getSuccess(game.fighter1.playerId)
			}

			if (game.fighter2.playerId !== -1) {
				ms2 = game.currentTurn.getElapsed(game.fighter2.playerId)
				s2 = game.currentTurn.getSuccess(game.fighter2.playerId)
			}

			_msec1.text = ms1 > 0 ? Number(ms1/1000).toLocaleString(Qt.locale(), "f", 3)+qsTr(" mp") : ""
			_msec2.text = ms2 > 0 ? Number(ms2/1000).toLocaleString(Qt.locale(), "f", 3)+qsTr(" mp") : ""

			if (ms1 > 0) {
				_icon1.success = s1
				_icon1.opacity = 1.0
			} else {
				_icon1.opacity = 0.0
			}

			if (ms2 > 0) {
				_icon2.success = s2
				_icon2.opacity = 1.0
			} else {
				_icon2.opacity = 0.0
			}
		}
	}
}
