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

	property int groupid: studentMaps ? studentMaps.selectedGroupId : -1
	property string username: cosClient.userName
	readonly property int recordsPerPage: Math.ceil(height/delegateHeight)+1


	toolBarComponent: QToolButton {
		action: actionCampaignFilter
		color: actionCampaignFilter.color
		display: AbstractButton.IconOnly
	}

	onRefreshRequest: reloadList(0)
	mode: TrophyView.Map


	QIconEmpty {
		visible: modelGameList.count == 0
		anchors.centerIn: parent
		textWidth: parent.width*0.75
		text: qsTr("Még nincs teljesített küldetés ebben a csoportban")
		tabContainer: control
	}

	Connections {
		target: studentMaps

		function onGameListUserReady(list, user, offset) {
			if (user === username) {
				fetchMoreSuccess()

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

	Connections {
		target: actionCampaignFilter

		function onCampaignChanged() {
			reloadList(0)
		}
	}


	function reloadList(_offset) {
		if (studentMaps) {
			if (actionCampaignFilter.campaign == -1)
				studentMaps.send("gameListUserGet", {
									 username: username,
									 groupid: groupid,
									 offset: _offset,
									 limit: recordsPerPage
								 })
			else
				studentMaps.send("gameListCampaignGet", {
									 username: username,
									 groupid: groupid,
									 campaignid: actionCampaignFilter.campaign,
									 offset: _offset,
									 limit: recordsPerPage
								 })
		} else
			modelGameList.clear()

	}

	onPopulated: {
		reloadList(0)
	}

	onFetchMoreRequest: {
		reloadList(modelGameList.count)
	}
}



