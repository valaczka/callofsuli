import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	defaultTitle: qsTr("Csoportok kezelése")

	activity: TeacherGroups {
		id: teacherGroups

		property VariantMapModel _dialogUserModel: newModel(["username", "firstname", "lastname", "active", "classname", "classid"])
		property VariantMapModel _dialogClassModel: newModel(["id", "name"])
		property VariantMapModel _dialogMapModel: newModel(["uuid", "name"])


		onGroupCreate: send("groupListGet")
		onGroupUpdate: {
			send("groupListGet")
			groupSelect(selectedGroupId)
		}

		onGroupClassAdd: {
			send("groupListGet")
			groupSelect(selectedGroupId)
		}

		onGroupClassRemove: {
			send("groupListGet")
			groupSelect(selectedGroupId)
		}

		onGroupRemove: {
			send("groupListGet")
			groupSelect(-1)
		}

		onGroupGet: {
			page.defaultSubTitle = jsonData.name
		}
	}



	property list<Component> cmps: [
		Component { TeacherGroupList {
				panelVisible: true
				Connections {
					target: page
					function onPageActivated() {
						list.forceActiveFocus()
					}
				}
			} },
		Component { TeacherGroupMembers {
				panelVisible: true
			} },
		Component { TeacherGroupMaps {
				panelVisible: true
			} }
	]




	/*mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}*/



	onPageActivated: {
		if (!panelComponents.length)
			panelComponents = cmps

		teacherGroups.send("groupListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
