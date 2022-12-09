import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	title: qsTr("Pályák kezelése")


	activity: TeacherMaps {
		id: teacherMaps

		onMapDownloadRequest: {
			if (teacherMaps.downloader.fullSize > cosClient.getSetting("autoDownloadBelow", 500000)) {
				var d = JS.dialogCreateQml("YesNo", {
											   title: qsTr("Letöltés"),
											   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize),
											   image: "qrc:/internal/icon/briefcase-download.svg"
										   })
				d.accepted.connect(function() {
					var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherMaps.downloader })
					dd.closePolicy = Popup.NoAutoClose
					dd.open()
				})

				d.open()
			} else {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			}
		}

		onMapAdd: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Hiba"), qsTr("Sikertelen feltöltés:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
			}
		}

		onMapModify: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Hiba"), qsTr("Sikertelen módosítás:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
			}
		}

		onMapRemove: {
			if (jsonData.error !== undefined) {
				cosClient.sendMessageWarningImage("qrc:/internal/icon/alert-outline.svg", qsTr("Hiba"), qsTr("Sikertelen törlés:\n%1").arg(jsonData.error))
			} else {
				send("mapListGet");
				control.stack.pop(null)
			}
		}
	}



	Component {
		id: cmpTeacherMapList
		TeacherMapList {
			onSelectMap: pushContent(cmpTeacherMapInfo)
		}
	}

	Component {
		id: cmpTeacherMapInfo
		TeacherMapInfo { }
	}



	Component.onCompleted: replaceContent(cmpTeacherMapList)

	onPageActivated: {
		teacherMaps.send("mapListGet", {})
	}
}

