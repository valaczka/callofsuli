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

		onGroupCreate: send("groupListGet")
		onGroupUpdate: send("groupListGet")
	}



	panelComponents: [
		Component { TeacherGroupList {
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
		teacherGroups.send("groupListGet")
	}



	function windowClose() {
		return false
	}


	function pageStackBack() {
		return false
	}

}
