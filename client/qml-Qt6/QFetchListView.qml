import QtQuick
import QtQuick.Controls
import Qaterial as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli

ListView {
	id: view

	property bool refreshEnabled: false
	property bool refreshProgressVisible: false

	property FetchModelImpl fetchModel: null
	property Flickable flickable: view

	property real defaultDelegateHeight: Qaterial.Style.delegate.implicitHeight(Qaterial.Style.DelegateType.Round, 2)
	property real fillHeight: -1

	readonly property int delegatePerFlickable: fillHeight > 0 && defaultDelegateHeight > 0 ?
													Math.ceil(fillHeight/defaultDelegateHeight) : -1

	property var reloadFunction: function() {
		if (fetchModel)
			fetchModel.reload()
	}


	onDelegatePerFlickableChanged: {
		if (delegatePerFlickable > 0)
			_limitTimer.restart()
	}

	Timer {
		id: _limitTimer
		interval: 250
		triggeredOnStart: false
		running: false
		onTriggered: {
			if (delegatePerFlickable > 0 && fetchModel)
				fetchModel.limit = Math.max(delegatePerFlickable+2, fetchModel.limit)
		}
	}


	focus: true
	activeFocusOnTab: true

	boundsBehavior: refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
	boundsMovement: Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	reuseItems: true

	ScrollIndicator.vertical: ScrollIndicator { }

	height: contentHeight

	model: fetchModel ? fetchModel.model : null

	footer: fetchModel && fetchModel.canFetch ? _cmpFooter : null

	Connections {
		target: flickable

		function onAtYEndChanged() {
			if (flickable.atYEnd && flickable.moving && fetchModel)
				fetchModel.fetch()
		}
	}

	Component {
		id: _cmpFooter

		Item {
			width: view.width
			height: Qaterial.Style.delegate.implicitHeight(Qaterial.Style.DelegateType.Icon, 1)

			Qaterial.Icon {
				anchors.centerIn: parent
				color: Qaterial.Style.iconColor()
				icon: Qaterial.Icons.reload
			}
		}
	}

	QRefreshProgressBar {
		id: progressbar
		anchors.top: parent.top
		visible: view.refreshProgressVisible
	}

	PullToRefreshHandler {
		target: view
		enabled: view.refreshEnabled && reloadFunction
		absoluteThreshold: view.height*0.65
		onPullDownRelease: reloadFunction()
	}

}
