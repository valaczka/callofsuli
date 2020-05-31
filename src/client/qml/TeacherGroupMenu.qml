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

	QLabel {
		id: groupName
		anchors.top: parent.top
		width: parent.width
		padding: 5
		topPadding: 10
		bottomPadding: 10
		elide: Text.ElideRight
		font.pixelSize: CosStyle.pixelSize*1.5
		font.weight: Font.Thin
		horizontalAlignment: Text.AlignHCenter
		verticalAlignment: Text.AlignVCenter

		visible: text.length
	}

	QListItemDelegate {
		id: list
		anchors.top: groupName.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

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
			groupName.text = name
		}
	}

	onPanelActivated: list.forceActiveFocus()

}
