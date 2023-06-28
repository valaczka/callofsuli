import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import CallOfSuli 1.0
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: control

	property LiteGame game: null
	property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")
	property string closeQuestion: qsTr("Biztosan megszakítod a játékot?")
	property var onPageClose: function() { if (game) game.gameAbort() }

	readonly property int stackViewIndex: StackView.index

	property bool itemsVisible: false

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
		visible: itemsVisible
	}



	GameHpLabel {
		id: infoHP
		anchors.horizontalCenter: parent.horizontalCenter
		anchors.topMargin: Math.max(5, Client.safeMarginTop)
		anchors.top: parent.top
		value: game ? game.hp : 0
		visible: itemsVisible
		onValueChanged: marked = true
	}


	Column {
		id: _colInfo
		anchors.right: parent.right
		anchors.top: parent.top
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)
		anchors.rightMargin: Math.max(Client.safeMarginRight, 7)
		spacing: 5

		visible: itemsVisible

		GameLabel {
			id: labelXP
			anchors.right: parent.right
			color: "white"

			pixelSize: 16

			text: "%1 XP"

			value: game ? game.xp : 0
		}

		GameInfo {
			id: infoTarget
			anchors.right: parent.right
			color: Qaterial.Colors.orange700
			iconLabel.icon.source: Qaterial.Icons.targetAccount
			text: Math.floor(progressBar.value)

			progressBar.from: 0
			progressBar.to: 0
			progressBar.value: enemies
			progressBar.width: Math.min(control.width*0.125, 100)

			property int enemies: game ? game.questions : 0

			onEnemiesChanged: {
				infoTarget.marked = true
				if (enemies>progressBar.to)
					progressBar.to = enemies
			}
		}
	}



	Row {
		id: rowTime

		visible: itemsVisible

		anchors.left: parent.left
		anchors.top: parent.top
		anchors.margins: 5
		anchors.leftMargin: Math.max(Client.safeMarginLeft, 10)
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)

		GameButton {
			id: backButton
			size: 25

			anchors.verticalCenter: parent.verticalCenter

			color: Qaterial.Colors.red800
			border.color: "white"
			border.width: 1

			fontImage.icon: Qaterial.Icons.close
			fontImage.color: "white"
			fontImageScale: 0.7

			onClicked: {
				Client.stackPop()
			}
		}

		GameLabel {
			id: infoTime
			color: Qaterial.Colors.cyan300

			anchors.verticalCenter: parent.verticalCenter

			iconLabel.icon.source: Qaterial.Icons.timerOutline

			iconLabel.text: game.msecLeft>=60000 ?
								Client.Utils.formatMSecs(game.msecLeft) :
								Client.Utils.formatMSecs(game.msecLeft, 1, false)
		}
	}




	Rectangle {
		id: blackRect
		anchors.fill: parent
		color: "black"
		visible: opacity
		opacity: 1.0
	}

	DropShadow {
		id: shadowName
		anchors.fill: nameLabel
		source: nameLabel
		color: "black"
		opacity: 0.0
		visible: opacity
		radius: 3
		samples: 7
		horizontalOffset: 3
		verticalOffset: 3
	}

	Label {
		id: nameLabel
		color: "white"
		font.family: "HVD Peace"
		font.pixelSize: Math.min(Math.max(30, (control.width/1000)*50), 60)
		text: game.name
		opacity: 0.0
		visible: opacity
		width: parent.width*0.7
		horizontalAlignment: Text.AlignHCenter
		x: (parent.width-width)/2
		y: (parent.height-height)/2
		wrapMode: Text.Wrap
	}




	GameQuestionAction {
		id: gameQuestion

		anchors.fill: parent
		anchors.topMargin: infoHP.y + 2*infoHP.pixelSize
		anchors.bottomMargin: Client.safeMarginBottom
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight

		game: control.game

		z: 5
	}


	Connections {
		target: game

		function onTimeNotify() {
			infoTime.marked = true
		}
	}


	state: "default"


	states: [
		State {
			name: "default"
			PropertyChanges {
				target: shadowName
				scale: 15.0
			}
			PropertyChanges {
				target: nameLabel
				scale: 15.0
			}
		},
		State {
			name: "start"
			PropertyChanges {
				target: control
				itemsVisible: true
			}
			PropertyChanges {
				target: shadowName
				scale: 1.0
			}
			PropertyChanges {
				target: nameLabel
				scale: 1.0
			}
			PropertyChanges {
				target: blackRect
				opacity: 0.0
			}
		},
		State {
			name: "run"
			PropertyChanges {
				target: blackRect
				opacity: 0.0
			}
			PropertyChanges {
				target: nameLabel
				opacity: 0.0
			}
			PropertyChanges {
				target: shadowName
				opacity: 0.0
			}
			PropertyChanges {
				target: control
				itemsVisible: true
			}
		}
	]


	transitions: [
		Transition {
			from: "default"
			to: "start"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						targets: [shadowName, nameLabel]
						property: "opacity"
						to: 1.0
						easing.type: Easing.InOutQuad;
						duration: 150
					}
					PropertyAnimation {
						targets: [shadowName, nameLabel]
						property: "scale"
						easing.type: Easing.InOutQuad;
						duration: 1000
					}
				}

				PropertyAction {
					target: control
					property: "itemsVisible"
				}

				PropertyAnimation {
					target: blackRect
					property: "opacity"
					easing.type: Easing.InExpo;
					duration: 700
				}

				ScriptAction {
					script: game.onPageReady()
				}

			}
		},
		Transition {
			from: "start"
			to: "run"
			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: shadowName
						property: "opacity"
						easing.type: Easing.InOutQuad;
						duration: 500
					}
					PropertyAnimation {
						target: nameLabel
						property: "opacity"
						easing.type: Easing.InOutQuad;
						duration: 1200
					}
				}

				ScriptAction {
					script: game.onStarted()
				}
			}
		}
	]


	StackView.onActivated: {
		state = "start"
	}

	Component.onCompleted: {
		if (!game) {
			console.error(qsTr("null game"))
			return
		}

	}


	function messageFinish(_text : string, _icon : string, _success : bool) {
		closeDisabled = ""
		closeQuestion = ""
		onPageClose = null
		Qaterial.DialogManager.showDialog(
					{
						onAccepted: function() { Client.stackPop(control) },
						onRejected: function() { Client.stackPop(control) },
						text: _text,
						title: qsTr("Game over"),
						iconSource: _icon,
						iconColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						textColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: Dialog.Ok
					})
	}

}
