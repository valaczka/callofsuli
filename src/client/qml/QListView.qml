import QtQuick 2.12
import QtQuick.Controls 2.12

ListView {
	id: view

	property int pauseDuration: 0

	clip: true
	focus: true
	boundsBehavior: Flickable.StopAtBounds

	ScrollIndicator.vertical: ScrollIndicator { }

	Keys.onLeftPressed: decrementCurrentIndex()
	Keys.onRightPressed: incrementCurrentIndex()

	height: contentHeight

/*
	add: Transition {
		id: dispTrans
		SequentialAnimation {
			PropertyAction {
				property: "opacity"
				value: 0
			}

			PauseAnimation {
				duration: pauseDuration + (dispTrans.ViewTransition.index -
										   dispTrans.ViewTransition.targetIndexes[0]) * 125
			}

			ParallelAnimation {
				NumberAnimation {
					property: "scale";
					duration: 250;
					from: 0.8;
					to: 1.0;
					easing.type: Easing.OutBack
				}
				NumberAnimation {
					property: "opacity"
					duration: 175
					easing.type: Easing.InQuad
					to: 1.0
				}
			}
		}
	}

*/

/*
	populate: Transition {
		id: popTrans
		SequentialAnimation {
			ScriptAction {
				script: {
					popTrans.ViewTransition.item.x = Math.random() > 0.5 ? width : -width
				}
			}

			PropertyAction {
				property: "opacity"
				value: 0
			}

			PauseAnimation {
				duration: pauseDuration + (popTrans.ViewTransition.index -
										   popTrans.ViewTransition.targetIndexes[0]) * 125
			}

			ParallelAnimation {
				NumberAnimation {
					property: "x";
					duration: 450;
					easing.type: Easing.OutBounce
				}
				NumberAnimation {
					property: "opacity"
					duration: 175
					easing.type: Easing.InQuad
					to: 1.0
				}
			}
		}
	}
*/

	/*
	remove: Transition {
		id: remTrans
		readonly property int toX: Math.random() > 0.5 ? width : -width

		ParallelAnimation {
			NumberAnimation {
				property: "scale";
				duration: 275;
				to: 0.2;
				easing.type: Easing.InQuart
			}
			NumberAnimation {
				property: "opacity"
				duration: 275
				easing.type: Easing.InExpo
				to: 0.0
			}
		}

	}
*/
}
