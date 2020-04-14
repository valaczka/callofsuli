import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: page

	header: QToolBar {
		id: toolbar
		title: cosClient.serverName

		backButton.visible: true

		rightLoader.sourceComponent: UserButton {
			userDetails: userData
		}
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPagePanel {
		id: panel

		anchors.fill: parent

		title: qsTr("Főmenü")
		maximumWidth: 600

		QListItemDelegate {
			id: list
			anchors.fill: parent

			model: ListModel { }

			onClicked: JS.createPage(model.get(index).page, model.get(index).params, page)
		}
	}

	UserDetails {
		id: userData
	}


	Connections {
		target: cosClient
		onUserRolesChanged: reloadModel()
	}


	StackView.onRemoved: {
		cosClient.closeConnection()
		destroy()
	}

	StackView.onActivated: {
		toolbar.resetTitle()
		reloadModel()
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function reloadModel() {
		list.model.clear()

		list.model.append({
							  labelTitle: qsTr("Rangsor"),
							  page: "Score",
							  params: {}
						  })

		if (cosClient.userRoles & Client.RoleTeacher)
			list.model.append({
								  labelTitle: qsTr("Pályák kezelése"),
								  page: "TeacherMaps",
								  params: {}
							  })


		if (cosClient.userRoles & Client.RoleAdmin)
			list.model.append({
								  labelTitle: qsTr("Felhasználók kezelése"),
								  page: "AdminUsers",
								  params: {}
							  })


		if (cosClient.userRoles & Client.RoleGuest)
			list.model.append({
								  labelTitle: qsTr("Bejelentkezés"),
								  page: "Login",
								  params: {}
							  })
	}


	function windowClose() {
		return true
	}


	function stackBack() {
		if (mainStack.depth > page.StackView.index+1) {
			if (!mainStack.get(page.StackView.index+1).stackBack()) {
				if (mainStack.depth > page.StackView.index+1) {
					mainStack.pop(page)
				}
			}
			return true
		}

		return false
	}
}
