import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	title: qsTr("Osztályok")

	QPageHeader {
		id: header

		isSelectorMode: classList.selectorSet

		labelCountText: classList.selectedItemCount

		QTextField {
			id: newClassName
			width: parent.width

			validator: RegExpValidator { regExp: /.+/ }

			placeholderText: qsTr("új osztály hozzáadása")
			onAccepted: {
				adminUsers.send({"class": "user", "func": "classCreate", "name": text })
			}
		}

		onSelectAll: classList.selectAll()
	}


	QListItemDelegate {
		id: classList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		isObjectModel: true
		modelTitleRole: "name"

		autoSelectorChange: true

		onClicked: pageAdminUsers.classSelected(model.get(index).id)
	}


	Connections {
		target: adminUsers

		onClassListLoaded: getClassList(list)

		onClassCreated: {
			newClassName.clear()
			reloadClassList()
		}

		onUserCreated: reloadUserList()
		onUserUpdated: reloadUserList()
	}

	onAdminUsersChanged: {
		if (adminUsers) {
			reloadClassList()
			reloadUserList()
		}
	}

	function reloadClassList() {
		adminUsers.send({"class": "user", "func": "getAllClass"})
	}


	function reloadUserList() {
		adminUsers.send({"class": "user", "func": "getAllUser"})
	}

	function getClassList(_list) {
		JS.setModel(classList.model, _list)
	}
}
