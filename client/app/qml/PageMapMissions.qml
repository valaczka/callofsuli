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
			JS.dialogMessage("success", qsTr("Game over"), qsTr("MISSION COMPLETED\nMegszerezve %1 XP\nTeljesítve: %2x\nStreak: %3/%4").arg(data.xp).arg(data.solved).arg(data.currentStreak).arg(data.maxStreak))
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

			visible: studentMaps && !studentMaps.demoMode && mapUuid.length

			icon.source: CosStyle.iconMedal

			onClicked: {
				var d = JS.dialogCreateQml("StudentMapInfo", {
											   title: page.defaultSubTitle,
											   studentMaps: studentMaps,
											   mapUuid: mapUuid
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
		}
	}

	Component.onDestruction: {
		if (_oldVisibility != null)
			mainWindow.visibility = _oldVisibility
	}


	onPageActivated: {
		cosClient.playSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
		if (studentMaps)
			studentMaps.getMissionList()
	}


	onPageDeactivated: {
		cosClient.stopSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
