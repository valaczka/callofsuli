import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import QtMultimedia 5.12
import QtQuick.Window 2.15
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property bool demoMode: false
	property string mapUuid: ""
	property bool readOnly: true

	property StudentMaps studentMaps: null

	compact: true



	Component {
		id: componentMissionLevel
		MapMissionLevel {
			studentMaps: control.studentMaps
			readOnly: control.readOnly
			actionLite: control.actionLite
		}
	}

	Component {
		id: componentMissionList
		MapMissionList {
			studentMaps: control.studentMaps
			onMissionLevelLoad: loadMissionLevel(uuid, level, deathmatch)
			actionLite: control.actionLite
		}
	}


	Component {
		id: demoStudentMapsComponent

		StudentMaps {
			Component.onCompleted: init(true)
		}
	}

	onDemoModeChanged: if (demoMode) {
						   studentMaps = demoStudentMapsComponent.createObject(control)
					   }


	onPageActivatedFirst: {
		if ((Qt.platform.os === "android" || Qt.platform.os === "ios") && height>width && cosClient.getSetting("notification/gameLandscape", true) === false) {
			cosClient.sendMessageInfo(qsTr("Képernyő tájolása"), qsTr("Fektesd el a képernyőt"))
			cosClient.setSetting("notification/gameLandscape", false)
		}
	}

	onPageActivated: {
		if (!actionLite.checked)
			cosClient.playSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
	}


	onPageDeactivated: {
		cosClient.stopSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
	}


	Connections {
		target: Qt.application
		function onStateChanged() {
			if (Qt.platform.os !== "android" && Qt.platform.os !== "ios")
				return

			switch (Qt.application.state) {
			case Qt.ApplicationSuspended:
			case Qt.ApplicationHidden:
				if (control.isCurrentItem)
					cosClient.stopSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
				break
			case Qt.ApplicationActive:
				if (control.isCurrentItem && !actionLite.checked)
					cosClient.playSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
				break
			}
		}
	}


	Component.onCompleted: {
		pushContent(componentMissionList)
	}


	function loadMissionLevel(uuid, level, deathmatch) {
		pushContent(componentMissionLevel, {
						missionUuid: uuid,
						missionLevel: level,
						missionDeathmatch: deathmatch
					})
	}


	property Action actionLite: Action {
		text: qsTr("Csak feladatok")
		checkable: true
		checked: true

		onToggled: {
			studentMaps.getMissionList(checked)
			if (checked)
				cosClient.stopSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
			else
				cosClient.playSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
		}
	}
}


