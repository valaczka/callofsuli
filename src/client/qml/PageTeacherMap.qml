import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Pályák kezelése")

	activity: TeacherMaps {
		id: teacherMaps

		onIsBusyChanged: {
			if (!isBusy && isUploading) {
				isUploading = false
				send("mapListGet")
			}
		}

		onMapDownloadRequest: {
			var d = JS.dialogCreateQml("YesNo", {
										   title: qsTr("Letöltés"),
										   text: qsTr("A szerver %1 adatot akar küldeni. Elindítod a letöltést?").arg(formattedDataSize)
									   })
			d.accepted.connect(function() {
				var dd = JS.dialogCreateQml("Progress", { title: qsTr("Letöltés"), downloader: teacherMaps.downloader })
				dd.closePolicy = Popup.NoAutoClose
				dd.open()
			})

			d.open()
		}

	}


	panelComponents: [
		Component { TeacherMapList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} }
	]




	/*mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}*/



	onPageActivated: {
		teacherMaps.send("mapListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
