import QtQuick 2.15
import QtQuick.Controls 2.15
import SortFilterProxyModel 0.2
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QBasePage {
	id: page

	defaultTitle: cosClient.serverName

	property Servers servers: null

	mainToolBarComponent: UserButton {
		userDetails: userData
		userNameVisible: page.width>800
	}

	UserDetails {
		id: userData
	}

	property VariantMapModel modelMenuList: cosClient.newModel([
																   "type",
																   "name",
																   "details",
																   "icon",
																   "page",
																   "id"
															   ])

	QStackComponent {
		anchors.fill: parent

		initialItem: QSimpleContainer {
			id: panel

			title: ""
			icon: "qrc:/internal/img/callofsuli.svg"

			borderColor: CosStyle.colorAccentDarker

			QVariantMapProxyView {
				id: menuList
				anchors.fill: parent

				model: SortFilterProxyModel {
					sourceModel: modelMenuList
					sorters: [
						RoleSorter { roleName: "type"; priority: 3 },
						RoleSorter { roleName: "id"; priority: 2 },
						StringSorter { roleName: "name"; priority: 1 }
					]

					proxyRoles: [
						SwitchRole {
							name: "textColor"
							filters: [
								ValueFilter {
									roleName: "type"
									value: 2										// Teacher
									SwitchRole.value: CosStyle.colorAccent
								},
								ValueFilter {
									roleName: "type"
									value: 3										// Admin
									SwitchRole.value: CosStyle.colorErrorLighter
								},
								ValueFilter {
									roleName: "type"
									value: 0										// Teacher/Student groups
									SwitchRole.value: CosStyle.colorOKLighter
								}
							]
							defaultValue: CosStyle.colorPrimaryLighter
						}
					]
				}


				modelTitleRole: "name"
				modelSubtitleRole: "details"
				modelTitleColorRole: "textColor"
				modelSubtitleColorRole: "textColor"

				autoSelectorChange: false

				refreshEnabled: true

				delegateHeight: CosStyle.twoLineHeight*1.3

				fontWeightTitle: Font.Medium
				pixelSizeTitle: CosStyle.pixelSize*1.2

				leftComponent: QFontImage {
					width: visible ? menuList.delegateHeight : 0
					height: width*0.8
					size: Math.min(height*0.8, 32)

					icon: model && model.icon ? model.icon : ""

					visible: model && model.icon

					color: model ? model.textColor : CosStyle.colorPrimary
				}


				onRefreshRequest: panel.reloadModel()

				onClicked: {
					var o = menuList.model.get(index)

					if (o.page.length) {
						var d = {}

						if (o.type === 0) {
							d.defaultTitle = o.name
							d.defaultSubTitle = o.details
							d.groupId = o.id
						}

						JS.createPage(o.page, d)
					}
				}
			}

			function reloadModel() {
				cosClient.socketSend(CosMessage.ClassUserInfo, "getMyGroups")
				menuList.forceActiveFocus()
			}

			Connections {
				target: cosClient
				function onUserRolesChanged(userRoles) {
					panel.reloadModel()
				}


				function onMyGroupListReady(_list) {
					var list = []

					list.push({
								  type: 1,
								  id: -10,
								  name: qsTr("Összesített rangsor"),
								  page: "Score",
								  details: "",
								  icon: CosStyle.iconTrophy
							  })

					if (!(cosClient.userRoles & Client.RoleGuest)) {
						list.push({
									  type: 1,
									  id: -11,
									  name: qsTr("Profil"),
									  page: "Profile",
									  details: "",
									  icon: CosStyle.iconUserWhite
								  })
					}

					if (cosClient.userRoles & CosMessage.RoleTeacher) {
						list.push({
									  type: 2,
									  id: -22,
									  name: qsTr("Pályák kezelése"),
									  page: "TeacherMap",
									  details: "",
									  icon: CosStyle.iconBooks
								  })

						list.push({
									  type: 2,
									  id: -21,
									  name: qsTr("Csoportok kezelése"),
									  page: "TeacherGroupEdit",
									  details: "",
									  icon: CosStyle.iconGroups
								  })


						list.push({
									  type: 4,
									  id: -41,
									  name: qsTr("Csatlakozás információk"),
									  page: "ConnectionInfo",
									  details: "",
									  icon: CosStyle.iconComputerData
								  })
					}


					if (cosClient.userRoles & CosMessage.RoleAdmin) {
						list.push({
									  type: 3,
									  id: -32,
									  name: qsTr("Szerver beállítása"),
									  page: "ServerSettings",
									  details: "",
									  icon: CosStyle.iconSetup
								  })

						list.push({
									  type: 3,
									  id: -31,
									  name: qsTr("Felhasználók kezelése"),
									  page: "AdminUsers",
									  details: "",
									  icon: CosStyle.iconUsers
								  })

					}


					if (cosClient.userRoles & CosMessage.RoleGuest) {
						list.push({
									  type: 4,
									  id: -43,
									  name: qsTr("Bejelentkezés"),
									  page: "Login",
									  details: "",
									  icon: CosStyle.iconLogin
								  })


						if (cosClient.registrationEnabled)
							list.push({
										  type: 4,
										  id: -42,
										  name: qsTr("Regisztráció"),
										  page: "Registration",
										  details: "",
										  icon: CosStyle.iconRegistration
									  })
					}



					for (var i=0; i<_list.length; i++) {
						var o = _list[i]

						list.push({
									  type: 0,
									  id: o.id,
									  name: o.name,
									  page: (cosClient.userRoles & CosMessage.RoleTeacher) ? "TeacherGroup" : "StudentGroup",
									  icon: (cosClient.userRoles & CosMessage.RoleTeacher) ? CosStyle.iconGroup : CosStyle.iconPlanet,
									  details: (o.readableClassList ? o.readableClassList : "")+
											   (o.teacherfirstname || o.teacherlastname ? " - " : "")+
											   (o.teacherfirstname ? o.teacherfirstname : "")+
											   (o.teacherlastname ? " "+o.teacherlastname : "")
								  })
					}

					modelMenuList.setVariantList(list, "id")
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
			cosClient.sendMessageWarning(qsTr("Bejelentkezés"), qsTr("A jelszó alaphelyzetben van. Adj meg egy új jelszót!"));
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

	onPageActivated: panel.reloadModel()

	onPageActivatedFirst: if (servers)
							  servers.serverTryLogin(servers.connectedServerKey)


	function windowClose() {
		return false
	}

	function pageStackBack() {
		return false
	}
}


