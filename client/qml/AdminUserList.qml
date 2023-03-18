import QtQuick 2.15
import QtQuick.Controls 2.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import SortFilterProxyModel 0.2
import "./JScript.js" as JS

Qaterial.Page
{
	id: control

	implicitWidth: 200
	implicitHeight: 200

	background: Rectangle { color: "transparent" }

	property alias view: view

	// Class id
	// -1: all
	// <-1: out of class

	property int classid: -1

	readonly property int _rolePanel: Credential.Panel
	readonly property int _roleAdmin: Credential.Admin
	readonly property int _roleTeacher: Credential.Teacher

	QListView {
		id: view

		currentIndex: 0
		anchors.fill: parent
		autoSelectChange: true

		refreshProgressVisible: Client.webSocket.pending
		refreshEnabled: true
		onRefreshRequest: reloadUsers()

		model: SortFilterProxyModel {
			sourceModel: userList

			filters: [
				ValueFilter {
					roleName: "classid"
					value: control.classid < -1 ? -1 : control.classid
					enabled: control.classid != -1
				}
			]

			sorters: [
				RoleSorter {
					roleName: "classid"
					priority: 2
					sortOrder: Qt.AscendingOrder
				},
				RoleSorter {
					roleName: "active"
					priority: 1
					sortOrder: Qt.DescendingOrder
				},
				StringSorter {
					roleName: "fullName"
					sortOrder: Qt.AscendingOrder
				}
			]

			proxyRoles: [
				SwitchRole {
					name: "userImage"
					filters: [
						ExpressionFilter {
							expression: model.roles & _rolePanel
							SwitchRole.value: Qaterial.Icons.desktopClassic
						},
						ExpressionFilter {
							expression: model.roles & _roleAdmin
							SwitchRole.value: Qaterial.Icons.accountTie
						},
						ExpressionFilter {
							expression: model.roles & _roleTeacher
							SwitchRole.value: Qaterial.Icons.accountTieHat
						}
					]
					defaultValue: Qaterial.Icons.account
				},
				ExpressionRole {
					name: "classNameReadable"
					expression: model.classid === -1 ? qsTr("Osztály nélkül") :
													   model.className.length ? model.className :
																				qsTr("Osztály #%1").arg(model.classid)
				}

			]
		}

		section.property: "classNameReadable"
		section.criteria: ViewSection.FullString
		section.delegate: Qaterial.ListSectionTitle {  }

		delegate: QItemDelegate {
			property User user: model.qtObject
			selectableObject: user

			highlighted: ListView.isCurrentItem
			iconSource: userImage
			text: user ? user.fullName: ""
			secondaryText: user ? user.username + (user.oauth.length ? " ("+user.oauth+")" : "") : ""
			textColor: user && user.active ? Qaterial.Style.colorTheme.primaryText : Qaterial.Style.colorTheme.disabledText
			iconColor: user ? (user.selected ? Qaterial.Style.accentColor :
											   user.active ? Qaterial.Style.iconColor() : Qaterial.Style.colorTheme.disabledText)
							: Qaterial.Style.colorTheme.primaryText



			onClicked: if (!view.selectEnabled)
						   Client.stackPushPage("AdminUserEdit.qml", {
													user: user
												})
		}

		Qaterial.Menu {
			id: contextMenu
			QMenuItem { action: view.actionSelectAll }
			QMenuItem { action: view.actionSelectNone }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionUserAdd }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionUserActivate }
			QMenuItem { action: actionUserInactivate }

			Qaterial.Menu {
				id: submenu
				title: qsTr("Áthelyez")

				Instantiator {
					model: SortFilterProxyModel {
						sourceModel: _moveMenuModel

						sorters: [
							FilterSorter {
								RangeFilter {
									roleName: "classid"
									maximumValue: 0
								}
								priority: 1
								sortOrder: Qt.AscendingOrder
							},
							StringSorter {
								roleName: "name"
								sortOrder: Qt.AscendingOrder
							}
						]
					}

					Qaterial.MenuItem {
						text: model.name
						onTriggered: {
							var l = view.getSelected()
							if (!l.length)
								return

							JS.questionDialogPlural(l, qsTr("Biztosan áthelyezed a kijelölt %1 felhasználót ide: ")+model.name+"?", "fullName",
													{
														onAccepted: function()
														{
															Client.send(WebSocket.ApiAdmin, "user/move/%1".arg(model.classid), {
																			list: JS.listGetFields(l, "username")
																		})
															.done(function(r){
																reloadUsers()
																_user.view.unselectAll()
															})
														},
														title: qsTr("Felhasználók áthelyezése"),
														iconSource: Qaterial.Icons.arrowRightBold
													})

						}
					}

					onObjectAdded: submenu.insertItem(index, object)
					onObjectRemoved: submenu.removeItem(object)
				}
			}
			QMenuItem { action: actionUserRemove }

		}

		onRightClickOrPressAndHold: {
			if (index != -1)
				currentIndex = index
			_moveMenuModel.reload()
			contextMenu.popup(mouseX, mouseY)
		}
	}

	QFabButton {
		visible: view.visible
		action: actionUserAdd
	}

	Action {
		id: actionUserRemove
		icon.source: Qaterial.Icons.minus
		text: qsTr("Törlés")
		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 felhasználót?"), "fullName",
									{
										onAccepted: function()
										{
											Client.send(WebSocket.ApiAdmin, "user/delete", {
															list: JS.listGetFields(l, "username")
														})
											.done(function(r){
												reloadUsers()
												_user.view.unselectAll()
											})

										},
										title: qsTr("Felhasználók törlése"),
										iconSource: Qaterial.Icons.closeCircle
									})

		}
	}

	Action {
		id: actionUserActivate
		icon.source: Qaterial.Icons.eye
		text: qsTr("Aktivál")

		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan aktiválod a kijelölt %1 felhasználót?"), "fullName",
									{
										onAccepted: function()
										{
											Client.send(WebSocket.ApiAdmin, "user/activate", {
															list: JS.listGetFields(l, "username")
														})
											.done(function(r){
												reloadUsers()
												_user.view.unselectAll()
											})
										},
										title: qsTr("Felhasználók aktiválása"),
										iconSource: Qaterial.Icons.eye
									})

		}
	}

	Action {
		id: actionUserInactivate
		icon.source: Qaterial.Icons.eyeOff
		text: qsTr("Inaktivál")

		onTriggered: {
			var l = view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan inaktiválod a kijelölt %1 felhasználót?"), "fullName",
									{
										onAccepted: function()
										{
											Client.send(WebSocket.ApiAdmin, "user/inactivate", {
															list: JS.listGetFields(l, "username")
														})
											.done(function(r){
												reloadUsers()
												_user.view.unselectAll()
											})
										},
										title: qsTr("Felhasználók inaktiválása"),
										iconSource: Qaterial.Icons.eyeOff
									})

		}
	}


	ListModel {
		id: _moveMenuModel

		function reload() {
			clear()

			append({classid: -1, name: qsTr("-- Osztály nélkül --")})

			for (var i=0; i<classList.length; i++)
				append(classList.get(i))

		}
	}

}
