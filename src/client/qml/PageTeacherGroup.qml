import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageTeacherGroup

	defaultTitle: qsTr("Csoportok kezelése")

	property alias panelMembers: panelMembers
	property alias panelMaps: panelMaps

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
	}



	panelComponents: [
		Component { TeacherGroupList {  } },
		Component { TeacherGroupMembers { } },
		Component { TeacherGroupMaps { } }
	]


	Component {
		id: panelMembers
		TeacherGroupMembers {}
	}

	Component {
		id: panelMaps
		TeacherGroupMaps {}
	}


	/*mainMenuFunc: function (m) {
		m.addAction(actionSave)
	}*/



	onPageActivated: {
		teacherGroups.send("groupListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
