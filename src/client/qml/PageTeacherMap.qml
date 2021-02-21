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

		onMapGet: {
			page.defaultSubTitle = jsonData.name
		}

		onMapRemove: {
			send("mapListGet")
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

	property list<Component> cmps: [
		Component { TeacherMapList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} },
		Component { TeacherMapGroups {
				panelVisible: true
			} }
	]




	/*mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}*/



	onPageActivated: {
		if (!panelComponents.length)
			panelComponents = cmps

		teacherMaps.send("mapListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
