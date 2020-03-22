import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

TrophyView {
	id: control

	title: qsTr("Eredmények")
	icon: CosStyle.iconXPgraph

	property int groupid: teacherGroups ? teacherGroups.selectedGroupId : -1
	property string username: ""
	readonly property int recordsPerPage: Math.ceil(height/delegateHeight)+1

	onRefreshRequest: reloadList(0)
	mode: username == "" ? TrophyView.User : TrophyView.Map


	QIconEmpty {
		visible: modelGameList.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Még nincs teljesített küldetés ebben a csoportban")
		tabContainer: control
	}

	Connections {
		target: teacherGroups

		function onGameListGroupReady(list, groupid, user, offset) {
			if (groupid !== control.groupid)
				return

			fetchMoreSuccess()

			if (username == "" || user === username) {
				if (offset)
					JS.listModelAppend(modelGameList, list)
				else
					JS.listModelReplace(modelGameList, list)

				if (list.length)
					canFetchMore = true
				else
					canFetchMore = false
			} else {
				modelGameList.clear()
				canFetchMore = false
			}
		}
	}


	function reloadList(_offset) {
		if (teacherGroups) {
			if (username == "")
				teacherGroups.send("gameListGroupGet", {
									   groupid: groupid,
									   offset: _offset,
									   limit: recordsPerPage
								   })
			else
				teacherGroups.send("gameListGroupGet", {
									   username: username,
									   groupid: groupid,
									   offset: _offset,
									   limit: recordsPerPage
								   })
		} else {
			modelGameList.clear()
			canFetchMore = false
		}
	}

	onPopulated: {
		reloadList(0)
	}

	onFetchMoreRequest: {
		reloadList(modelGameList.count)
	}
}



