import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import "JScript.js" as JS

Page {
	id: root

	property RpgGame game: null
	property int currentDrop: -1
	property var drops: []

	Rectangle {
		anchors.fill: parent
		color: Qaterial.Colors.black
	}

	Rectangle {
		id: _drop

		width: 300
		height: 500

		anchors.centerIn: parent

		state: switch (tier) {
			   case 0: return "common";
			   case 1: return "uncommon";
			   case 2: return "rare";
			   case 3: return "epic";
			   case 4: return "legendary";
			   default:
				   return ""
			   }

		color: "transparent"

		property int tier: 0
		property int realTier: 0
		property int xp: 0
		property int token: 0
		property int point: 0
		property int click: 0

		function load(_data) {
			_result.visible = false
			click = 0
			tier = 0
			realTier = _data.tier

			xp = _data.xp
			point = _data.point
			token = _data.token

			_startTimer.start()
		}

		function next(_byTimer) {

			if (_byTimer)
				click = Math.max(click, tier+1)
			else
				++click

			if (tier < realTier)
				++tier
			else if (click > 4 || _byTimer) {
				if (_result.visible)
					loadNextDrop()
				else
					_result.visible = true
			}
		}


		Column {
			id: _result
			visible: false

			anchors.centerIn: parent

			Qaterial.LabelBody1 {
				text: _drop.xp+" XP"
				color: "black"
			}

			Qaterial.LabelBody1 {
				text: _drop.point+" point"
				color: "black"
			}

			Qaterial.LabelBody1 {
				text: _drop.token+" TOKEN"
				color: "black"
			}
		}

		/*
		Common = 0,
		Uncommon,
		Rare,
		Epic,
		Legendary
		*/

		SequentialAnimation {
			loops: Animation.Infinite
			ScaleAnimator {
				target: _drop
				from: 1
				to: 1.3
				duration: 750
				easing.type: Easing.OutBack
			}
			ScaleAnimator {
				target: _drop
				from: 1.3
				to: 1
				duration: 750
				easing.type: Easing.InBack
			}
		}

		states: [
			State {
				name: "common"
				PropertyChanges {
					_drop.color: "white"
				}
			},
			State {
				name: "uncommon"
				PropertyChanges {
					_drop.color: "yellow"
				}
			},
			State {
				name: "rare"
				PropertyChanges {
					_drop.color: "red"
				}
			},
			State {
				name: "epic"
				PropertyChanges {
					_drop.color: "green"
				}
			},
			State {
				name: "legendary"
				PropertyChanges {
					_drop.color: "blue"
				}
			}
		]


		transitions: [
			Transition {
				from: "*"
				to: "*"

				ParallelAnimation {
					ColorAnimation { duration: 200 }
					PropertyAnimation {
						target: _drop
						properties: "y,x"
						easing.type: Easing.OutInElastic
						easing.amplitude: 2.0
						easing.period: 1.5
					}
				}
			}

		]

	}

	Timer {
		id: _startTimer
		interval: 5000
		triggeredOnStart: false
		repeat: false
		running: false
		onTriggered: {
			openCurrent()
			_timer.start()
		}
	}

	Timer {
		id: _timer
		interval: 1250
		triggeredOnStart: true
		running: false
		repeat: true
		onTriggered: _drop.next(true)
	}

	Timer {
		id: _timerNext
		interval: 2000
		triggeredOnStart: false
		running: false
		onTriggered: {
			if (currentDrop >= drops.length)
				finish()
			else
				_drop.load(drops[currentDrop])
		}
	}

	MouseArea {
		anchors.fill: parent
		onClicked: {
			if (_startTimer.running) {
				_startTimer.stop()
				_timer.restart()
				openCurrent()
			} else {
				_drop.next(false)
			}
		}
	}

	Qaterial.AppBarButton
	{
		id: _backButton
		anchors.left: parent.left
		anchors.leftMargin: Client.safeMarginLeft
		anchors.top: parent.top
		anchors.topMargin: Client.safeMarginTop
		icon.source: Qaterial.Icons.arrowLeft

		onClicked: Client.stackPop()
	}


	StackView.onDeactivating: {

	}

	StackView.onActivated: {
		drops = game.rpgUserData.drops

		loadNextDrop()
	}

	function openCurrent() {
		console.debug("OPEN", currentDrop)
	}

	function finish() {
		_timer.stop()
		Client.stackPop()
	}

	function loadNextDrop() {
		_timer.stop()

		if (!game) {
			finish()
			return
		}

		if (_timerNext.running) {
			return
		}

		++currentDrop

		if (currentDrop >= drops.length) {
			_timerNext.start()
			return
		}

		if (currentDrop == 0)
			_drop.load(drops[currentDrop])
		else {
			_timerNext.start()
		}
	}

}
