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
	boundsMovement: Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	ScrollIndicator.vertical: ScrollIndicator { }

	Keys.onLeftPressed: decrementCurrentIndex()
	Keys.onRightPressed: incrementCurrentIndex()

	height: contentHeight

	signal refreshRequest()

	onDragStarted: if (refreshEnabled)
					   header.visible = true

	onDragEnded: {
		if (refreshEnabled && header.refresh) {
			header.readyToRefreshRequest = true
		}
	}


	onVerticalOvershootChanged: {
		if (refreshEnabled && header.visible && verticalOvershoot>=0 && header.readyToRefreshRequest) {
			refreshRequest()
			header.readyToRefreshRequest = false
		}

		if (refreshEnabled && verticalOvershoot==0) {
			header.visible = false
		}
	}

	Item {
		id: header
		y: header.readyToRefreshRequest ?
			   Math.max(-height-verticalOvershoot, headerItem ? headerItem.height : 0)
			 : -height-verticalOvershoot
		height: -view.refreshActivateY
		width: parent.width
		visible: false

		readonly property bool refresh: state == "pulled" ? true : false
		property bool readyToRefreshRequest: false

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
				name: "base"; when: view.verticalOvershoot >= (refreshActivateY-(headerItem ? headerItem.height : 0))
				PropertyChanges { target: arrow; rotation: 180 }
			},
			State {
				name: "pulled"; when: view.verticalOvershoot < (refreshActivateY-(headerItem ? headerItem.height : 0)) || header.readyToRefreshRequest
				PropertyChanges { target: arrow; rotation: 0 }
				PropertyChanges { target: arrow; color: CosStyle.colorAccentLighter }
			}
		]
	}


}
