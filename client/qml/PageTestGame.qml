import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS


Page {
	id: control

	property TestGame game: null
	property string closeDisabled: qsTr("A játék előkészítése alatt nem lehet bezárni a lapot!")
	property string closeQuestion: qsTr("Biztosan megszakítod a játékot?")
	property var onPageClose: function() { if (game) game.gameAbort() }

	property bool itemsVisible: false
	readonly property bool resultVisible: game && game.hasResult

	onResultVisibleChanged: gameQuestion.visible = !resultVisible

	Image {
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/internal/img/villa.png"
		visible: itemsVisible

		cache: true
	}

	Row {
		id: _buttonRow

		anchors.right: parent.right
		anchors.top: parent.top
		anchors.margins: 5
		anchors.rightMargin: Math.max(Client.safeMarginRight, 10)
		anchors.topMargin: Math.max(Client.safeMarginTop, 5)

		visible: itemsVisible && !resultVisible

		enabled: control.game && (control.game.finishState == AbstractGame.Invalid || control.game.finishState == AbstractGame.Neutral)


		/*Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.stopCircle
			enabled: game
			onClicked: game.finishGame()
		}*/

		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.arrowLeftCircleOutline
			enabled: game && game.currentQuestion > 0
			onClicked: game.previousQuestion()
		}

		Qaterial.AppBarButton {
			icon.source: Qaterial.Icons.arrowRightCircleOutline
			enabled: game && game.currentQuestion < game.questions
			onClicked: game.nextQuestion()
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



	QScrollable {
		anchors.fill: parent

		topPadding: Math.max (_buttonRow.y + _buttonRow.height, rowTime.y+rowTime.height)
		bottomPadding: Client.safeMarginBottom
		rightPadding: Client.safeMarginRight
		leftPadding: Client.safeMarginLeft

		visible: resultVisible

		contentCentered: true
		refreshEnabled: false

		GameTestResult {
			id: _testResult
			anchors.horizontalCenter: parent.horizontalCenter
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

			visible: !resultVisible

			anchors.verticalCenter: parent.verticalCenter

			iconLabel.icon.source: Qaterial.Icons.timerOutline

			iconLabel.text: game.msecLeft>=60000 ?
								Client.Utils.formatMSecs(game.msecLeft) :
								Client.Utils.formatMSecs(game.msecLeft, 1, false)
		}
	}



	GameQuestionTest {
		id: gameQuestion

		game: control.game

		enabled: control.game && (control.game.finishState == AbstractGame.Invalid || control.game.finishState == AbstractGame.Neutral)

		anchors.top: _buttonRow.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		anchors.bottomMargin: Math.max(Client.safeMarginBottom, _progress.height)
		anchors.leftMargin: Client.safeMarginLeft
		anchors.rightMargin: Client.safeMarginRight
	}


	Qaterial.ProgressBar {
		id: _progress

		visible: !resultVisible

		anchors.bottom: parent.bottom
		width: parent.width
		color: Qaterial.Style.iconColor()
		from: 0
		to: game ? game.questions : 0
		value: game ? game.currentQuestion : 0
	}




	Connections {
		target: game

		function onTimeNotify() {
			infoTime.marked = true
		}

		function onResultChanged() {
			game.resultToQuickTextDocument(_testResult.textDocument)
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
						/*onAccepted: function() { Client.stackPop(control) },
						onRejected: function() { Client.stackPop(control) },*/
						text: _text,
						title: qsTr("Game over"),
						iconSource: _icon,
						iconColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						textColor: _success ? Qaterial.Colors.green500 : Qaterial.Colors.red500,
						iconFill: false,
						iconSize: Qaterial.Style.roundIcon.size,
						standardButtons: DialogButtonBox.Ok
					})
	}

}
