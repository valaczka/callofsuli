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

		property VariantMapModel modelMapList: newModel([
															"rowid",
															"uuid",
															"name",
															"version",
															"binded",
															"used",
															"lastModified",
															"dataSize",
															"editLocked"
														])

		onMapListGet: {
			modelMapList.unselectAll()
			modelMapList.setVariantList(jsonData.list, "rowid");
		}

		onMapCreate: {
			if (jsonData.created)
				send("mapListGet")
		}

		onMapEditLock: {
			if (jsonData.locked) {
				teacherMaps.editUuid = jsonData.uuid
				JS.createPage("MapEditor", {
								  parentActivity: teacherMaps
							  })
			}
		}

		onMapEditUnlock: {
			if (jsonData.unlocked && jsonData.uuid === teacherMaps.editUuid) {
				teacherMaps.editUuid = ""
			}
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
