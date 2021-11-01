import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import QtGraphicalEffects 1.0
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS



QBasePage {
	id: control

	defaultTitle: ""
	defaultSubTitle: ""

	property StudentMaps studentMaps: null
	property string username: ""
	property int groupid: -1



	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		initialItem: TrophyView {
			id: panel
			onRefreshRequest: reloadList()
			mode: TrophyView.Map
		}
	}


	Connections {
		target: studentMaps

		function onGameListUserReady(list, user) {
			if (user === username)
				panel.modelGameList.replaceList(list)
			else
				panel.modelGameList.clear()
		}
	}


	onPageActivated: reloadList()

	function reloadList() {
		if (studentMaps)
			studentMaps.send("gameListUserGet", {
								   username: username,
								   groupid: groupid
							   })
		else
			panel.modelGameList.clear()

	}

	function windowClose() {
		return false
	}

	function pageStackBack() {
		if (stackComponent.layoutBack())
			return true

		return false
	}



}

