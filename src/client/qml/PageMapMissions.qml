import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtMultimedia 5.12
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	property bool demoMode: false

	property var _oldVisibility: null

	property StudentMaps studentMaps: null

	StudentMaps {
		id: demoStudentMaps

		demoMode: true

		onGamePlayReady: {
			var o = JS.createPage("Game", {
									  gameMatch: gameMatch,
									  deleteGameMatch: true
								  })
		}

	}

	onDemoModeChanged: if (demoMode) {
						   demoStudentMaps.client = cosClient
						   studentMaps = demoStudentMaps
					   }

	mainToolBarComponent: UserButton {
		visible: studentMaps && !studentMaps.demoMode
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	property list<Component> cmps: [
		Component { MapMissionList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} },
		Component { MapMissionInfo {
				panelVisible: true
			} }
	]


	/*mainMenuFunc: function (m) {
			m.addAction(actionSave)
		}*/


	Component.onCompleted: {
		_oldVisibility = mainWindow.visibility
		mainWindow.visibility = "FullScreen"
	}

	Component.onDestruction: {
		if (_oldVisibility != null)
			mainWindow.visibility = _oldVisibility
	}


	onPageActivated: {
		cosClient.playSound("qrc:/sound/menu/bg.ogg", CosSound.Music)
		if (!panelComponents.length)
			panelComponents = cmps
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
