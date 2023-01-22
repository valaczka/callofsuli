import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Controls.Material 2.12
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial

ListView {
	id: view

	property int pauseDuration: 0
	property bool refreshEnabled: false
	property bool refreshProgressVisible: false
	property alias areaRightButtonEnabled: areaRightButton.enabled
	property bool selectEnabled: false
	property bool autoSelectChange: false

	signal refreshRequest()
	signal rightClickOrPressAndHold(int index, int mouseX, int mouseY)


	property Action actionSelectAll: Action {
		text: qsTr("Mindet")
		icon.source: Qaterial.Icons.selectAll
		onTriggered: {
			for (var i=0; i<view.count; ++i)
				view.model.get(i).selected = true

			view.selectEnabled = true
		}
	}

	property Action actionSelectNone: Action {
		text: qsTr("Semmit")
		icon.source: Qaterial.Icons.selectOff
		onTriggered: {
			for (var i=0; i<view.count; ++i)
				view.model.get(i).selected = false

			if (view.autoSelectChange)
				view.selectEnabled = false
		}
	}


	clip: true
	focus: true
	activeFocusOnTab: true

	boundsBehavior: refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
	boundsMovement: Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	reuseItems: true

	ScrollIndicator.vertical: ScrollIndicator { }

	Keys.onLeftPressed: decrementCurrentIndex()
	Keys.onRightPressed: incrementCurrentIndex()

	height: contentHeight

	Qaterial.ProgressBar
	{
		id: progressbar
		anchors.top: parent.top
		width: parent.width
		indeterminate: true
		visible: view.refreshProgressVisible
		color: Qaterial.Style.iconColor()
	}

	PullToRefreshHandler {
		absoluteThreshold: 100
		onPullDownRelease: view.refreshRequest()
	}


	/*

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

		Qaterial.RoundColorIcon {
			id: arrow
			anchors.horizontalCenter: parent.horizontalCenter
			anchors.top: parent.top
			source: Qaterial.Icons.refresh
			iconSize: 24 * Qaterial.Style.pixelSizeRatio
			roundSize: 40 * Qaterial.Style.pixelSizeRatio
			transformOrigin: Item.Center
			fill: true
			onPrimary: true
			Behavior on rotation { NumberAnimation { duration: 200 } }
		}


		states: [
			State {
				name: "base"; when: view.verticalOvershoot >= (refreshActivateY-(headerItem ? headerItem.height : 0))
				PropertyChanges { target: arrow; rotation: 180 }
			},
			State {
				name: "pulled"; when: view.verticalOvershoot < (refreshActivateY-(headerItem ? headerItem.height : 0))
									  || header.readyToRefreshRequest
				PropertyChanges { target: arrow; rotation: 0 }
				//PropertyChanges { target: arrow; color: CosStyle.colorAccentLighter }
			}
		]
	}

*/
	MouseArea {
		id: areaRightButton
		anchors.fill: parent
		acceptedButtons: Qt.RightButton
		onClicked: {
			view.rightClickOrPressAndHold(parent.indexAt(mouse.x, mouse.y), mouse.x, mouse.y)
		}
	}



	function selectAll() {
		actionSelectAll.trigger()
	}


	function unselectAll() {
		actionSelectNone.trigger()
	}

	function checkSelected() {
		for (var i=0; i<count; ++i)
			if (model.get(i).selected)
				return

		if (autoSelectChange)
			selectEnabled = false
	}
}
