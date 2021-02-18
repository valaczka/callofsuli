import QtQuick 2.15
import QtQuick.Controls 2.15
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: page

	requiredPanelWidth: 900

	title: cosClient.serverName

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	swipeMode: control.width < 900

	panelComponents: Component {
		QPagePanel {
			panelVisible: true
			layoutFillWidth: true

			id: panel

			title: qsTr("Főmenü")
			maximumWidth: 600

			QMenuItemDelegate {
				id: list
				anchors.fill: parent

				model: ListModel { }

				onClicked: JS.createPage(model.get(index).page, model.get(index).params)
			}

			onPanelActivated: reloadModel()

			onPopulated: reloadModel()


			function reloadModel() {
				list.model.clear()

				list.model.append({
									  labelTitle: qsTr("Rangsor"),
									  page: "Score",
									  params: {}
								  })

				if (cosClient.userRoles & CosMessage.RoleStudent)
					list.model.append({
										  labelTitle: qsTr("Pályák"),
										  page: "StudentMap",
										  params: {}
									  })

				if (cosClient.userRoles & CosMessage.RoleTeacher)
					list.model.append({
										  labelTitle: qsTr("Pályák kezelése"),
										  page: "TeacherMap",
										  params: {}
									  })

				if (cosClient.userRoles & CosMessage.RoleTeacher)
					list.model.append({
										  labelTitle: qsTr("Csoportok kezelése"),
										  page: "TeacherGroup",
										  params: {}
									  })

				if (cosClient.userRoles & CosMessage.RoleAdmin)
					list.model.append({
										  labelTitle: qsTr("Szerver beállításai"),
										  page: "ServerSettings",
										  params: {}
									  })

				if (cosClient.userRoles & CosMessage.RoleAdmin)
					list.model.append({
										  labelTitle: qsTr("Felhasználók kezelése"),
										  page: "AdminUsers",
										  params: {}
									  })


				if (cosClient.userRoles & CosMessage.RoleGuest)
					list.model.append({
										  labelTitle: qsTr("Bejelentkezés"),
										  page: "Login",
										  params: {}
									  })


				list.forceActiveFocus()
			}

			Connections {
				target: cosClient
				function onUserRolesChanged(userRoles) {
					reloadModel()
				}
			}

			Connections {
				target: page
				function onPageActivated() {
					list.forceActiveFocus()
				}
			}

		}
	}



	Connections {
		target: cosClient
		function onUserRolesChanged(userRole) {
			if (page.StackView.view)
				mainStack.pop(page)
		}

		function onAuthRequirePasswordReset() {
			mainStack.pop(page)
			JS.createPage("PasswordReset", {})
		}

		function onRegistrationRequest() {
			mainStack.pop(page)
			JS.createPage("Registration", {})
		}
	}

	StackView.onRemoved: {
		cosClient.closeConnection()
	}


	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}
}


