import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QTabPage {
	id: control

	property alias groupId: teacherGroups.selectedGroupId

	title: teacherGroups.selectedGroupFullName

	//buttonColor: CosStyle.colorPrimary
	buttonBackgroundColor: Qt.darker("#006400")

	buttonModel: ListModel {
		id: modelGuest

		ListElement {
			title: qsTr("Pályák")
			icon: "image://font/School/\uf19d"
			iconColor: "lime"
			func: function() { replaceContent(cmpTeacherGroupMapList) }
		}
		ListElement {
			title: qsTr("Résztvevők")
			icon: "image://font/School/\uf154"
			iconColor: "orchid"
			func: function() { replaceContent(cmpTeacherGroupUserList) }
			checked: true
		}
		ListElement {
			title: qsTr("Eredmények")
			icon: "image://font/AcademicI/\uf15d"
			iconColor: "tomato"
			func: function() { replaceContent(cmpTeacherGroupScore) }
		}
	}


	activity: TeacherGroups {
		id: teacherGroups

		onGroupRemove: {
			if (jsonData.error === undefined) {
				var i = mainStack.get(control.StackView.index-1)

				if (i)
					mainStack.pop(i)
			}
		}


		onGroupModify: {
			if (jsonData.error === undefined && jsonData.id === selectedGroupId)
				send("groupGet", {id: selectedGroupId})
		}
	}


	Component {
		id: cmpTeacherGroupMapList
		TeacherGroupMapList { }
	}

	Component {
		id: cmpTeacherGroupUserList
		TeacherGroupUserList { }
	}

	Component {
		id: cmpTeacherGroupScore
		TeacherGroupScore { }
	}


	onPageActivated: {
		if (teacherGroups.selectedGroupId > -1) {
			teacherGroups.send("groupGet", {id: teacherGroups.selectedGroupId})
		} else {
			teacherGroups.modelMapList.clear()
		}
	}

}

