import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageStudentMap

	property string mapUuid: ""
	property string mapMd5: ""

	property bool _isFirstRun: true
	property bool _isMapLoaded: false

	property alias student: map.student

	property alias map: map

	signal campaignSelected(int cid)
	signal missionDataSelected(var mdata)

	subtitle: cosClient.serverName

	mainToolBarComponent: QToolBusyIndicator { running: map.student && map.student.isBusy }

	StudentMap {
		id: map
		client: cosClient

		/*void mapLoadingStarted(const QString &uuid);
		void mapDownloadRequest(const QString &uuid);
		void mapDownloaded(const QString &uuid);*/

		onMapDownloadError: {
			var d = JS.dialogMessageError(qsTr("Megnyitás"), qsTr("Nem sikerült letölteni és megnyitni a pályát!"))
			d.onClosedAndDestroyed.connect(function() {
				mainStack.back()
			})
		}

		onMapLoadingProgress: {
			progressBar.value = progress
		}

		onMapLoaded: {
			_isMapLoaded = true
			reloadResult()
		}
	}


	Item {
		id: loadingItem
		anchors.fill: parent

		visible: !_isMapLoaded

		ProgressBar {
			id: progressBar
			anchors.centerIn: parent
		}
	}

	panels: [
		{ url: "MapCampaignList.qml", params: { studentMap: map }, fillWidth: false},
		{ url: "MapMissionList.qml", params: { studentMap: map }, fillWidth: false},
		{ url: "MapMission.qml", params: { studentMap: map }, fillWidth: true}
	]

	panelsVisible: _isMapLoaded


	onCampaignSelected: swipeToPage(1)
	onMissionDataSelected: swipeToPage(2)

	onPageActivated: {
		if (_isFirstRun) {
			if (mapUuid.length)
				map.loadFromRepository(mapUuid, mapMd5)

			_isFirstRun = false
		}
	}


	function playGame(mId, isSum, lev) {
		var o = JS.createPage("Game", {})
		o.pagePopulated.connect(function() {
			o.game.map = map
			o.game.missionId = mId
			o.game.isSummary = isSum
			o.game.level = lev
			o.game.gamePlayMode = Game.GamePlayOnline
			o.game.prepare()
		})
	}

	function reloadResult() {
		student.send({"class": "student", "func": "getMapResult", "uuid": mapUuid})
	}

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}

}
