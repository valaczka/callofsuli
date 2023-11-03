import QtQuick
import QtQuick.Controls
import Qt5Compat.GraphicalEffects
import CallOfSuli
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial

GameQuestionBase {
	id: control

	property int horizontalPadding: control.width>control.height ? 40 : 5
	property int verticalPadding: 5


	readonly property bool _isFullscreen: (control.width>control.height && (Qt.platform.os == "android"  || Qt.platform.os === "ios")) || control.height < 800

	panel.width: _isFullscreen ? control.width-4 :
								 Math.min(contentLoader.item ? contentLoader.item.implicitWidth : 400, control.width-2*horizontalPadding)
	panel.height: _isFullscreen ? control.height-4 :
								  Math.min(contentLoader.item ? contentLoader.item.implicitHeight : 300, control.height-2*verticalPadding)
	panel.x: (control.width-panel.width)/2
	panel.y: (control.height-panel.height)/2


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
				PropertyAction {
					target: control
					property: "visible"
					value: true
				}

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
					script: {
						control.onShowAnimationFinished()
					}
				}

			}
		},
		Transition {
			from: "started"
			to: "finished"

			SequentialAnimation {
				PauseAnimation {
					duration: control.msecBeforeHide
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
						control.onHideAnimationFinished()

						/*if (questionPrivate && questionPrivate.mode == GameMatch.ModeNormal) {
								cosClient.playSound("qrc:/sound/sfx/question.mp3")
							}*/
					}
				}

			}
		}
	]

}
