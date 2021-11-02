import QtQuick 2.15
import QtQuick.Controls 2.15
import "Style"

ListView {
	id: view

	property int pauseDuration: 0
	property int refreshActivateY: -50
	property bool refreshEnabled: false

	clip: true
	focus: true
	activeFocusOnTab: true
	boundsBehavior: refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	ScrollIndicator.vertical: ScrollIndicator { }

	Keys.onLeftPressed: decrementCurrentIndex()
	Keys.onRightPressed: incrementCurrentIndex()

	height: contentHeight

	signal refreshRequest()

	onDragStarted: if (refreshEnabled)
					   header.visible = true
	onDragEnded: {
		if (refreshEnabled && header.refresh)
			refreshRequest()
		header.visible = false
	}

	Item {
		id: header
		y: -view.contentY - height
		height: -view.refreshActivateY
		width: parent.width
		visible: false

		property bool refresh: state == "pulled" ? true : false

		QFontImage {
			id: arrow
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: parent.top
			icon: CosStyle.iconRefresh
			width: CosStyle.pixelSize*1.5
			height: CosStyle.pixelSize*1.5
			size: CosStyle.pixelSize*1.5
			color: CosStyle.colorPrimaryLighter
			transformOrigin: Item.Center
			Behavior on rotation { NumberAnimation { duration: 200 } }
		}


		states: [
			State {
				name: "base"; when: view.contentY >= refreshActivateY
				PropertyChanges { target: arrow; rotation: 180 }
			},
			State {
				name: "pulled"; when: view.contentY < refreshActivateY
				PropertyChanges { target: arrow; rotation: 0 }
				PropertyChanges { target: arrow; color: CosStyle.colorAccentLighter }
			}
		]
	}




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
