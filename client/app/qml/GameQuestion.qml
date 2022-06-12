import QtQuick 2.15
import QtQuick.Controls 2.15
import QtGraphicalEffects 1.0
import QtMultimedia 5.12
import COS.Client 1.0
import "Style"
import "JScript.js" as JS

Item {
	id: control

	implicitWidth: 500
	implicitHeight: 200

	anchors.fill: parent

	property GameQuestionPrivate questionPrivate: null

	signal succeed(real xpFactor)
	signal failed()

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property int horizontalPadding: control.width>control.height ? 40 : 5
	property int verticalPadding: 5

	property bool isAnswerSuccess: false

	enabled: false

	focus: true

	Keys.onPressed: if (contentLoader.item)
						contentLoader.item.keyPressed(event.key)

	Item {
		id: panel

		readonly property bool _isFullscreen: (control.width>control.height && (Qt.platform.os == "android"  || Qt.platform.os === "ios")) || control.height < 800

		width: _isFullscreen ? control.width-4 :
							   Math.min(contentLoader.item ? contentLoader.item.implicitWidth : 400, control.width-2*horizontalPadding)
		height: _isFullscreen ? (questionPrivate && questionPrivate.mode != GameMatch.ModeNormal ? control.height-65 : control.height-4) :
								Math.min(contentLoader.item ? contentLoader.item.implicitHeight : 300, control.height-2*verticalPadding)
		x: (control.width-width)/2
		y: questionPrivate && questionPrivate.mode != GameMatch.ModeNormal ? 60 : (control.height-height)/2

		opacity: 0.1
		scale: 0.1

		DropShadow {
			anchors.fill: panel
			horizontalOffset: 3
			verticalOffset: 3
			color: JS.setColorAlpha("black", 0.75)
			source: border2
		}

		BorderImage {
			id: border2
			source: "qrc:/internal/img/border2.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 10
			border.left: 5
			border.right: 80
			border.bottom: 10

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}


		Image {
			id: metalbg
			source: "qrc:/internal/img/metalbg.png"
			visible: false
			fillMode: Image.Tile
			anchors.fill: panel
		}

		OpacityMask {
			id: opacity1
			anchors.fill: panel
			source: metalbg
			maskSource: border2
		}


		FocusScope {
			id: realContent
			anchors.fill: parent
			visible: true

			anchors.topMargin: 5
			anchors.leftMargin: 0
			anchors.rightMargin: 0
			anchors.bottomMargin: 10

			opacity: 0.0

			Loader {
				id: contentLoader
				anchors.fill: parent

				sourceComponent: Item {
					id: defaultItem

					implicitHeight: 300
					implicitWidth: 400

					signal succeed()
					signal failed()
					signal postponed()

					GameQuestionButton {
						anchors.centerIn: parent
						text: "X"
						onClicked: {
							interactive = false
							defaultItem.failed()
						}
					}
				}
			}

			Connections {
				target: contentLoader.item

				function onSucceed() {
					if (questionPrivate && questionPrivate.mode == GameMatch.ModeNormal) {
						cosClient.playSound("qrc:/sound/sfx/correct.mp3", CosSound.GameSound)
						cosClient.playSound("qrc:/sound/voiceover/winner.mp3", CosSound.VoiceOver)
					}
					isAnswerSuccess = true
					succeed(questionPrivate.questionData().xpFactor)
					control.state = "finished"
				}

				function onFailed() {
					if (questionPrivate && questionPrivate.mode == GameMatch.ModeNormal) {
						cosClient.playSound("qrc:/sound/voiceover/loser.mp3", CosSound.VoiceOver)
					}
					isAnswerSuccess = false
					failed()
					control.state = "finished"
				}

				function onPostponed() {
					if (questionPrivate.postpone()) {
						isAnswerSuccess = true
						control.state = "finished"
					}
				}

				function onAnswered(answer) {
					console.debug("ON ANSWERED", answer)
					questionPrivate.answer = answer
					isAnswerSuccess = true
					control.state = "finished"
				}
			}

		}





		// BORDER

		BorderImage {
			id: border1
			source: "qrc:/internal/img/border1.svg"
			visible: false

			//sourceSize.height: 141
			//sourceSize.width: 414

			anchors.fill: panel
			border.top: 15
			border.left: 10
			border.right: 60
			border.bottom: 25

			horizontalTileMode: BorderImage.Repeat
			verticalTileMode: BorderImage.Repeat
		}

		ColorOverlay {
			anchors.fill: border1
			source: border1
			color: borderColor
		}

	}

	onQuestionPrivateChanged: {
		if (!questionPrivate)
			return

		var q = questionPrivate.questionQml()
		if (q && q.length) {
			contentLoader.setSource(q, {
										questionData: questionPrivate.questionData(),
										mode: questionPrivate ? questionPrivate.mode : GameMatch.ModeNormal,
										canPostpone: questionPrivate ? questionPrivate.canPostpone : false
									})
		} else {
			cosClient.sendMessageError(qsTr("Belső hiba"), qsTr("Érvénytelen kérdés"))
		}

	}

	Component.onCompleted: {
		state = "started"
	}

	states: [
		State {
			name: "started"

			PropertyChanges {
				target: panel
				opacity: 1.0
				scale: 1.0
			}

			PropertyChanges {
				target: realContent
				opacity: 1.0
			}
		},

		State {
			name: "finished"

			PropertyChanges {
				target: panel
				opacity: 0.0
				scale: 0.0
			}

			PropertyChanges {
				target: realContent
				opacity: 0.0
			}
		}
	]

	transitions: [
		Transition {
			from: "*"
			to: "started"

			SequentialAnimation {
				ParallelAnimation {
					PropertyAnimation {
						target: panel
						property: "opacity"
						duration: 235
						easing.type: Easing.OutQuad
					}

					PropertyAnimation {
						target: panel
						property: "scale"
						duration: 275
						easing.type: Easing.OutBack
						easing.overshoot: 3
					}
				}

				PropertyAnimation {
					target: realContent
					property: "opacity"
					duration: 175
				}

				ScriptAction {
					script: if (questionPrivate && questionPrivate.mode == GameMatch.ModeNormal) {
								cosClient.playSound("qrc:/sound/sfx/question.mp3")
							}
				}

				PropertyAction {
					target: control
					property: "enabled"
					value: true
				}
			}
		},
		Transition {
			from: "started"
			to: "finished"

			SequentialAnimation {
				PauseAnimation {
					duration: isAnswerSuccess ? 0 : 1250
				}

				ParallelAnimation {
					PropertyAnimation {
						target: panel
						property: "opacity"
						duration: 125
						easing.type: Easing.InQuad
					}

					PropertyAnimation {
						target: panel
						property: "scale"
						duration: 125
						easing.type: Easing.InQuad
						easing.overshoot: 3
					}

					PropertyAnimation {
						target: realContent
						property: "opacity"
						duration: 125
						easing.type: Easing.InQuad
					}
				}

				ScriptAction {
					script: {
						control.destroy()
					}
				}

			}
		}
	]

}
