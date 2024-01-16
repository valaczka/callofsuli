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
				view.modelGet(i).selected = true

			view.selectEnabled = true
		}
	}

	property Action actionSelectNone: Action {
		text: qsTr("Semmit")
		icon.source: Qaterial.Icons.selectOff
		onTriggered: {
			for (var i=0; i<view.count; ++i)
				view.modelGet(i).selected = false

			if (view.autoSelectChange)
				view.selectEnabled = false
		}
	}

	focus: true
	activeFocusOnTab: true

	boundsBehavior: refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
	boundsMovement: Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	reuseItems: true

	ScrollIndicator.vertical: ScrollIndicator { }

	Keys.onLeftPressed: decrementCurrentIndex()
	Keys.onRightPressed: incrementCurrentIndex()

	onCurrentIndexChanged: {
		if (currentIndex != -1)
			positionViewAtIndex(currentIndex, ListView.Contain)
	}


	height: contentHeight

	QRefreshProgressBar {
		id: progressbar
		anchors.top: parent.top
		visible: view.refreshProgressVisible
	}

	PullToRefreshHandler {
		target: view
		enabled: view.refreshEnabled
		absoluteThreshold: view.height*0.65
		onPullDownRelease: view.refreshRequest()
	}


	MouseArea {
		id: areaRightButton
		anchors.fill: parent
		acceptedButtons: Qt.RightButton
		onClicked: {
			// Ha van header, akkor nem jó indexet ad vissza. +originY korrekció kell, de csak oda
			view.rightClickOrPressAndHold(view.indexAt(mouse.x, mouse.y+view.originY), mouse.x, mouse.y)
		}
	}


	function modelGet(_index) {
		if (model.sourceModel)
			return model.sourceModel.get(model.mapToSource(_index))
		else
			return model.get(_index)
	}

	function selectAll() {
		actionSelectAll.trigger()
	}


	function unselectAll() {
		actionSelectNone.trigger()
	}

	function checkSelected() {
		for (var i=0; i<count; ++i)
			if (modelGet(i).selected)
				return

		if (autoSelectChange)
			selectEnabled = false
	}

	function getSelected() {
		var l = []

		if (selectEnabled) {
			for (var i=0; i<count; ++i) {
				var o = modelGet(i)
				if (o.selected)
					l.push(o)
			}
		} else if (currentIndex != -1) {
			l.push(modelGet(currentIndex))
		}

		return l
	}
}
