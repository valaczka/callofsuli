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
	property string fileToOpen: ""

	property StudentMaps studentMaps: null

	compact: true



	Component {
		id: componentMissionLevel
		MapMissionLevel {
			studentMaps: control.studentMaps
			readOnly: control.readOnly
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
			Component.onCompleted: {
				init(true, fileToOpen)
			}
		}
	}


	onPageActivatedFirst: {
		if ((Qt.platform.os === "android" || Qt.platform.os === "ios") && height>width && cosClient.getSettingBool("notification/gameLandscape", true) === true) {
			cosClient.sendMessageInfoImage("qrc:/internal/icon/phone-rotate-landscape.svg", qsTr("Képernyő tájolása"), qsTr("Fektesd el a képernyőt"))
			cosClient.setSetting("notification/gameLandscape", false)
		}
	}

	onPageActivated: {
		if (studentMaps && !studentMaps.liteMode)
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
				if (control.isCurrentItem && studentMaps && !studentMaps.liteMode)
					cosClient.playSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
				break
			}
		}
	}


	Component.onCompleted: {
		if (demoMode) {
			studentMaps = demoStudentMapsComponent.createObject(control)
		}
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
		text: studentMaps && studentMaps.liteMode ?
				  qsTr("Normál játék") :
				  qsTr("Csak feladatok")

		icon.source: studentMaps && studentMaps.liteMode ?
						 "qrc:/internal/icon/pistol.svg" :
						 "qrc:/internal/icon/file-edit-outline.svg"

		onTriggered: {
			studentMaps.liteMode = !studentMaps.liteMode
			studentMaps.getMissionList()
			if (studentMaps.liteMode)
				cosClient.stopSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
			else
				cosClient.playSound("qrc:/sound/menu/bg.mp3", CosSound.Music)
		}
	}
}


