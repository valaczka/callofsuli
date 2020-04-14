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

		height: col.height

		Column {
			id: col
			width: parent.width


			QTextField {
				id: newClassName
				width: parent.width

				placeholderText: qsTr("új osztály hozzáadása")
				onAccepted: {
					console.debug("send "+text)
					adminUsers.send({"class": "user", "func": "classCreate", "name": text })
				}
			}
		}
	}

	QListItemDelegate {
		id: classList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		onClicked: reloadUserList()
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


	function reloadClassList() {
		adminUsers.send({"class": "user", "func": "getAllClass"})
	}


	function reloadUserList() {
		var d = {"class": "user", "func": "getAllUser"}

		var mode = -5

		if (classList.currentIndex != -1) {
			mode = classList.model.get(classList.currentIndex).id
		}

		if (mode === -1) {
			d.classid = null
			d.isTeacher = false
		} else if (mode === -2) {
			d.isTeacher = false
		} else if (mode === -3) {
			d.isTeacher = true
		} else if (mode === -4) {
			d.isAdmin = true
		} else if (mode === -5) {

		} else {
			d.classid = mode
			d.isTeacher = false
		}

		if (mode >= 0)
			pageAdminUsers.classSelected(mode)
		else
			pageAdminUsers.classSelected(-1)

		/*if (active !== null)
			d.active = active*/

		adminUsers.send(d)
	}



	function getClassList(_list) {
		classList.model.clear()

		for (var i=0; _list && i<_list.length; ++i) {
			var o=_list[i]
			o.labelTitle = o.name
			classList.model.append(o)
		}

		classList.model.append({
							  id: -1,
							  labelTitle: qsTr("Osztály nélkül")
						  })

		classList.model.append({
							  id: -2,
							  labelTitle: qsTr("Minden diák")
						  })

		classList.model.append({
							  id: -3,
							  labelTitle: qsTr("Tanárok")
						  })

		classList.model.append({
							  id: -4,
							  labelTitle: qsTr("Adminok")
						  })

		classList.model.append({
							  id: -5,
							  labelTitle: qsTr("Mindenki")
						  })


	}
}
