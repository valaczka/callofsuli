import QtQuick 2.12
import QtQuick.Controls 2.12
import QtGraphicalEffects 1.0
import "Style"
import "JScript.js" as JS


Popup {
	id: popupItem

	closePolicy: Popup.CloseOnEscape
	modal: true
	focus: true

	property alias effectSource: effectsource.sourceItem
	property alias popupContent: popupContent
	property alias item: popupContent.item

	signal accepted(var data)
	signal rejected()
	signal closedAndDestroyed()

	background: Item {
		anchors.fill: parent
		ShaderEffectSource {
			id: effectsource
			anchors.fill: parent
			sourceRect: Qt.rect(0,0,width, height)
			visible: false
		}

		FastBlur {
			id: blurEffect
			anchors.fill: effectsource
			source: effectsource
			radius: 0
			visible: false
		}

		BrightnessContrast {
			id: brightnessEffect
			anchors.fill: blurEffect
			source: blurEffect
			brightness: 0
			visible: true
		}
	}


	enter: Transition {
		SequentialAnimation {
			ParallelAnimation {
				PropertyAnimation {
					target: blurEffect
					property: "radius"
					to: 75
					duration: 225
					easing.type: Easing.InOutQuad
				}

				PropertyAnimation {
					target: brightnessEffect
					property: "brightness"
					to: -0.5
					duration: 225
					easing.type: Easing.InOutQuad
				}
			}

			ParallelAnimation {
				PropertyAnimation {
					target: popupContent
					property: "opacity"
					from: 0.0
					to: 1.0
					duration: 235
					easing.type: Easing.OutQuad
				}

				PropertyAnimation {
					target: popupContent
					property: "scale"
					from: 0.0
					to: 1.0
					duration: 275
					easing.type: Easing.OutBack
					easing.overshoot: 3
				}
			}

			ScriptAction {
				script: popupContent.item.populated()
			}
		}
	}

	exit: Transition {
		SequentialAnimation {
			PropertyAnimation {
				target: popupContent
				properties: "opacity, scale"
				from: 1.0
				to: 0.0
				duration: 125
				easing.type: Easing.InQuad
			}

			ParallelAnimation {
				PropertyAnimation {
					target: blurEffect
					property: "radius"
					to: 0
					duration: 125
					easing.type: Easing.InOutQuad
				}

				PropertyAnimation {
					target: brightnessEffect
					property: "brightness"
					to: 0
					duration: 125
					easing.type: Easing.InOutQuad
				}
			}

			ScriptAction {
				script: {
					popupItem.closedAndDestroyed()
					popupContent.sourceComponent = undefined
					popupContent.source = ""
					popupItem.destroy()
				}
			}
		}
	}


	Loader {
		id: popupContent
		anchors.fill: parent
		opacity: 0.0
	}

	Connections {
		target: popupContent.item
		function onDlgClose() {
			close()
		}
	}


	onAboutToHide: {
		if (popupContent.item.acceptedData)
			accepted(popupContent.item.acceptedData)
		else
			rejected()
	}

}
