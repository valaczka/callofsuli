import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	title: qsTr("Felhasználók")

	QPageHeader {
		id: header

		height: row.height

		Row {
			id: row
			QButton {
				label: "Új"

				onClicked: {
					pageAdminUsers.userSelected("")
				}
			}
		}
	}

	QListItemDelegate {
		id: userList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		modelSubtitleSet: true

		onClicked: pageAdminUsers.userSelected(model.get(index).username)
	}


	Connections {
		target: adminUsers

		onUserListLoaded: getList(list)
	}


	function getList(_list) {
		userList.model.clear()

		for (var i=0; _list && i<_list.length; ++i) {
			var o=_list[i]
			o.labelTitle = o.firstname+" "+o.lastname
			o.labelSubtitle = o.username
			userList.model.append(o)
		}

	}
}
