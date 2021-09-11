import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	implicitWidth: 1200

	property bool demoMode: false
	property string mapUuid: ""

	property var _oldVisibility: null

	//stackMode: true

	property StudentMaps studentMaps: null

	defaultTitle: qsTr("Küldetések")

	StudentMaps {
		id: demoStudentMaps

		demoMode: true

		onGamePlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  studentMaps: demoStudentMaps,
									  deleteGameMatch: true
								  })

			isGameRunning = o ? true : false
		}

		onGameFinishDialogReady: {
			var d = JS.dialogCreateQml("MissionCompleted", { gameData: data, demoMode: true })
			d.accepted.connect(function(m) {
				if (demoStudentMaps.gamePage) {
					cosClient.sendMessageError(qsTr("Belső hiba"), qsTr("Már folyamatban van egy játék!"))
				} else {
					demoStudentMaps.playGame({
											 uuid: m.missionid,
											 level: m.level,
											 deathmatch: m.deathmatch
										 })
				}
			})
			d.open()
		}
	}

	Connections {
		target: studentMaps

		function onMissionSelected(index) {
			if (stackMode && index !== -1) {
				var o = addStackPanel(panelInfo)
				o.selectedMissionIndex = index
				o.loadMission()
			}
		}
	}

	onDemoModeChanged: if (demoMode) {
						   demoStudentMaps.client = cosClient
						   studentMaps = demoStudentMaps
					   }

	mainToolBarComponent: Row {
		QToolButton {
			anchors.verticalCenter: parent.verticalCenter
			ToolTip.text: qsTr("Jelvények")

			visible: studentMaps

			icon.source: CosStyle.iconMedal

			onClicked: {
				var d = JS.dialogCreateQml("StudentMapInfo", {
											   title: page.defaultSubTitle,
											   studentMaps: studentMaps
										   })
				d.open()

			}
		}

		UserButton {
			anchors.verticalCenter: parent.verticalCenter
			visible: studentMaps && !studentMaps.demoMode
			userDetails: userData
			userNameVisible: page.width>800
		}
	}

	UserDetails {
		id: userData
	}

	Component {
		id: panelInfo
		MapMissionInfo { }
	}

	panelComponents: [
		Component { MapMissionList { } },
		Component { MapMissionInfo { } }
	]



	Component.onCompleted: {
		if (Qt.platform.os === "android") {
			_oldVisibility = mainWindow.visibility
			mainWindow.visibility = "FullScreen"
			cosClient.forceLandscape()
		}
	}

	Component.onDestruction: {
		if (_oldVisibility != null)
			mainWindow.visibility = _oldVisibility

		if (Qt.platform.os === "android") {
			cosClient.resetLandscape()
		}
	}


	onPageActivated: {
		cosClient.playSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
		if (studentMaps)
			studentMaps.getMissionList()
	}


	onPageDeactivated: {
		cosClient.stopSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
	}


	Connections {
		target: Qt.application
		function onStateChanged() {
			if (Qt.platform.os !== "android")
				return


			switch (Qt.application.state) {
			case Qt.ApplicationSuspended:
			case Qt.ApplicationHidden:
				if (page.isCurrentItem)
					cosClient.stopSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
				break
			case Qt.ApplicationActive:
				if (page.isCurrentItem)
					cosClient.playSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
				break
			}
		}
	}

	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
