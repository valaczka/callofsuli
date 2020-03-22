import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtMultimedia 5.12
import QtQuick.Window 2.15
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: page

	property bool demoMode: false
	property string mapUuid: ""

	//property var _oldVisibility: null

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

	onDemoModeChanged: if (demoMode) {
						   demoStudentMaps.client = cosClient
						   studentMaps = demoStudentMaps
					   }

	property int _currentLevel: -1
	property bool _currentDeathmatch: false
	property bool _currenPageIsInfo: false
	property int _currenMissionIndex: -1


	mainToolBarComponent: Row {
		QToolButton {
			id: buttonInfo
			anchors.verticalCenter: parent.verticalCenter
			icon.source: CosStyle.iconXPgraph

			visible: !demoMode && _currenPageIsInfo && _currentLevel > 0

			ToolTip.text: qsTr("Eredmények")
			onClicked: {
				var o = stack.pushComponent(panelUserView, {
												missionIndex: _currenMissionIndex,
												level: _currentLevel,
												deathmatch: _currentDeathmatch
											})
				o.reloadData()
			}
		}

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

	Component {
		id: panelUserView
		MapMissionUserView { }
	}


	QStackComponent {
		id: stack
		anchors.fill: parent

		requiredWidth: 1200

		initialItem: MapMissionList {}

		Connections {
			target: studentMaps

			function onMissionSelected(index) {
				var o = stack.pushComponent(panelInfo, {
												selectedMissionIndex: index,
												basePage: page
											})
				o.loadMission()
				_currenMissionIndex = index
			}
		}


	}



/*
	Component.onCompleted: {
		if (Qt.platform.os === "android" || Qt.platform.os === "ios") {
			_oldVisibility = mainWindow.visibility
			mainWindow.visibility = "FullScreen"
		}
	}

	Component.onDestruction: {
		if (_oldVisibility != null)
			mainWindow.visibility = _oldVisibility
	}
*/

	onPageActivatedFirst: {
		if ((Qt.platform.os === "android" || Qt.platform.os === "ios") && height>width) {
			cosClient.sendMessageInfo(qsTr("Képernyő tájolása"), qsTr("Fektesd el a képernyőt"))
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
		if (stack.layoutBack())
			return true

		return false
	}

}
