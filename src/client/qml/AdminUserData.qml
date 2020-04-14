import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null
	property string username: ""

	property int selectedClass: -1

	title: qsTr("Felhasználó adatai")


	QGridLayout {
		id: grid

		width: parent.width
		watchModification: true

		QGridLabel { field: textUserName }

		QGridTextField {
			id: textUserName
			fieldName: username.length ? qsTr("Felhasználónév") : qsTr("Új felhasználó létrehozása")
			sqlField: "username"

			readOnly: username.length

			validator: RegExpValidator { regExp: /[A-Za-z0-9_-.]+/ }
		}


		QGridLabel { field: textFirstName }

		QGridTextField {
			id: textFirstName
			fieldName: qsTr("Vezetéknév")
			sqlField: "firstname"
		}


		QGridLabel { field: textLastName }

		QGridTextField {
			id: textLastName
			fieldName: qsTr("Keresztnév")
			sqlField: "lastname"
		}


		QGridLabel { field: textEmail }

		QGridTextField {
			id: textEmail
			fieldName: qsTr("E-mail")
			sqlField: "email"

			validator: RegExpValidator { regExp: /^(.+@..+\...+|)$/ }
		}

		QGridCheckBox {
			id: checkActive
			text: qsTr("Aktív")
			sqlField: "active"
		}

		QGridLabel { text: qsTr("Osztály") }

		QGridComboBox {
			id: comboClass
			sqlField: "classid"

			model: ListModel { }

			textRole: "text"
			valueRole: "value"
		}

		QGridCheckBox {
			id: checkTeacher
			text: qsTr("Tanár")
			sqlField: "isTeacher"
		}

		QGridCheckBox {
			id: checkAdmin
			enabled: checkTeacher.checked && username !== cosClient.userName
			text: qsTr("Admin")
			sqlField: "isAdmin"
		}

		QGridButton {
			id: buttonSave
			label: username.length ? qsTr("Mentés") : qsTr("Létrehozás")
			disabled: !textUserName.acceptableInput ||
					  !textFirstName.acceptableInput ||
					  !textLastName.acceptableInput ||
					  !textEmail.acceptableInput ||
					  !grid.modified

			onClicked: {
				var m = JS.getSqlFields([textUserName, textFirstName, textLastName, textEmail, checkActive, checkTeacher, checkAdmin, comboClass],
										!username.length)


				if (Object.keys(m).length) {
					if (username.length)
						m.username = username

					m["class"] = "user"

					if (username.length)
						m.func = "userUpdate"
					else
						m.func = "userCreate"

					adminUsers.send(m)
				}
			}

		}
	}


	Connections {
		target: adminUsers

		onUserLoaded: load(data)

		onUserUpdated: if (data.error) {
						   cosClient.sendMessageWarning(tr("Felhasználó módosítása"), data.error)
					   } else {
						   username=data.updatedUserName
					   }

		onUserCreated: {
			if (data.error) {
				cosClient.sendMessageWarning(qsTr("Felhasználó létrehozása"), data.error)
			} else {
				username=data.createdUserName
			}
		}

		onClassListLoaded: {
			comboClass.model.clear()

			comboClass.model.append({text: qsTr("Osztály nélkül"), value: -1})

			for (var i=0; list && i<list.length; ++i) {
				var o=list[i]
				comboClass.model.append({text: o.name, value: o.id})
			}
		}
	}

	Connections {
		target: pageAdminUsers

		onUserSelected: username = name

		onClassSelected: {
			selectedClass = id
			username = ""
			load()
		}
	}

	onUsernameChanged: get()

	Component.onCompleted: {
		adminUsers.send({"class": "user", "func": "getAllClass"})
		get()
	}

	function get() {
		if (username.length)
			adminUsers.send({"class": "user", "func": "userGet", "username": username})
		else
			load()
	}


	function load(d) {
		textUserName.setText(d && d.username ? d.username : "")
		textFirstName.setText(d && d.firstname ? d.firstname : "")
		textLastName.setText(d && d.lastname ? d.lastname: "")
		textEmail.setText(d && d.email ? d.email : "")
		checkActive.setChecked(d && d.active ? d.active : false)
		checkTeacher.setChecked(d && d.isTeacher ? d.isTeacher : false)
		checkAdmin.setChecked(d && d.isAdmin ? d.isAdmin : false)

		if (d)
			comboClass.setValue(d.classid ? d.classid : -1)
		else
			comboClass.setValue(selectedClass)
	}
}
