import QtQuick 2.12
import QtQuick.Controls 2.12
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

	property var questionData: null

	signal successSound()
	signal succeed()
	signal failed()

	property bool isAnswerSuccess: false

	property color borderColor: CosStyle.colorPrimaryDarker
	property color titleColor: CosStyle.colorAccentLighter

	property int horizontalPadding: 20
	property int verticalPadding: 10


	Item {
		id: panel

		width: Math.min(contentLoader.item ? contentLoader.item.implicitWidth : 400, control.width-2*horizontalPadding)
		height: Math.min(contentLoader.item ? contentLoader.item.implicitHeight : 300, control.width-2*verticalPadding)
		x: (control.width-width)/2
		y: (control.height-height)/2

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


		Item {
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
			}

			Connections {
				target: contentLoader.item

				function onSucceed() {
					control.successSound()
					isAnswerSuccess = true
					control.state = "finished"
				}

				function onFailed() {
					isAnswerSuccess = false
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

	onQuestionDataChanged: {
		if (questionData && questionData.module.length) {
			contentLoader.setSource("GameQuestion_"+questionData.module+".qml", {
										questionData: questionData
									})
		}
	}

	Component.onCompleted: state = "started"

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
					script: cosClient.playSound("qrc:/sound/sfx/question.ogg")
				}
			}
		},
		Transition {
			from: "started"
			to: "finished"

			SequentialAnimation {
				PauseAnimation {
					duration: isAnswerSuccess ? 250 : 1250
				}

				ParallelAnimation {
					PropertyAnimation {
						target: panel
						property: "opacity"
						duration: 235
						easing.type: Easing.InQuad
					}

					PropertyAnimation {
						target: panel
						property: "scale"
						duration: 275
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
						if (isAnswerSuccess)
							succeed()
						else
							failed()
					}
				}

			}
		}
	]

}
