import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property Teacher teacher: null
	property int groupId: -1


	title: qsTr("Lehetőségek")
	maximumWidth: 600

	Item {
		id: selectItem
		anchors.fill: parent

		visible: groupId == -1

		QLabel {
			anchors.centerIn: parent
			text: qsTr("Válassz csoportot")
		}
	}

	QListItemDelegate {
		id: list
		anchors.fill: parent

		modelTitleRole: "title"

		visible:  groupId != -1

		model: ListModel {
			ListElement {
				title: qsTr("Szerkesztés")
				page: "TeacherGroupEdit"
			}
			ListElement {
				title: qsTr("Members")
				page: "TeacherGroupMembers"
			}
		}

		onClicked: {
			var m = model.get(index)
			JS.createPage(m.page, {groupId: groupId, teacher: teacher})
		}
	}

	Connections {
		target: pageTeacherGroup
		onGroupSelected: {
			groupId = id
			panel.title = name
		}
	}

	onPanelActivated: list.forceActiveFocus()

}
