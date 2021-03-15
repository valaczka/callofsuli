import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Pályák kezelése")

	activity: TeacherMaps {
		id: teacherMaps

		property VariantMapModel _dialogGroupModel: newModel(["id", "name", "readableClassList"])

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

		onMapRemove: {
			send("mapListGet")
		}

		function mapSelect(uuid) {
			teacherMaps.selectedMapId=uuid
			if (uuid !== "" && stackMode)
				addStackPanel(panelMapGroups)
		}
	}


	FileDialog {
		id: fileDialog
		title: qsTr("Exportálás")
		folder: shortcuts.home

		property string mapUuid: ""

		selectMultiple: false
		selectExisting: false

		onAccepted: {
			teacherMaps.mapExport({uuid: mapUuid, filename: fileDialog.fileUrl})
		}
	}

	panelComponents: [
		Component { TeacherMapList {  } },
		Component { TeacherMapGroups { } }
	]

	Component {
		id: panelMapGroups
		TeacherMapGroups { }
	}


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
