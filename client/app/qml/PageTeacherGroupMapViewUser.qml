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

	property TeacherGroups teacherGroups: null
	property string username: ""
	property int groupid: -1
	property string mapUuid: ""


	QStackComponent {
		id: stackComponent
		anchors.fill: parent
		basePage: control

		initialItem: TrophyView {
			id: panel
			onRefreshRequest: reloadList()
			mode: TrophyView.Mission
		}
	}


	Connections {
		target: teacherGroups

		function onGameListMapReady(list, map, user) {
			if (map !== mapUuid || user !== username)
				panel.modelGameList.clear()
			else
				panel.modelGameList.replaceList(list)
		}
	}


	onPageActivated: reloadList()

	function reloadList() {
		if (teacherGroups)
			teacherGroups.send("gameListMapGet", {
								   username: username,
								   mapid: mapUuid
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

