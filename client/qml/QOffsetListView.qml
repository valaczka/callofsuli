import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0

ListView {
	id: view

	property bool refreshEnabled: false
	property bool refreshProgressVisible: false

	property OffsetModelImpl offsetModel: OffsetModelImpl {  }
	/*property alias fields: offsetModel.fields
	property alias api: offsetModel.api
	property alias apiData: offsetModel.apiData
	property alias path: offsetModel.path
	property alias limit: offsetModel.limit
	property alias listField: offsetModel.listField*/

	focus: true
	activeFocusOnTab: true

	boundsBehavior: refreshEnabled ? Flickable.DragAndOvershootBounds : Flickable.StopAtBounds
	boundsMovement: Flickable.StopAtBounds

	highlightFollowsCurrentItem: false

	reuseItems: true

	ScrollIndicator.vertical: ScrollIndicator { }

	height: contentHeight

	model: offsetModel ? offsetModel.model : null

	footer: offsetModel && offsetModel.canFetch ? _cmpFooter : null

	onAtYEndChanged: {
		if (atYEnd && moving && offsetModel)
			offsetModel.fetch()
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
		enabled: view.refreshEnabled
		absoluteThreshold: view.height*0.65
		onPullDownRelease: if (offsetModel) offsetModel.reload()
	}

	function reload() {
		if (offsetModel)
			offsetModel.reload()
	}
}
