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

	background: null

	property alias view: view

	// Class id
	// -1: all
	// <-1: out of class

	property int classid: -1
	property string classname: ""
	property string classcode: ""


	readonly property int _rolePanel: Credential.Panel
	readonly property int _roleAdmin: Credential.Admin
	readonly property int _roleTeacher: Credential.Teacher

	QScrollable
	{
		anchors.fill: parent

		Qaterial.TextField {
			id: textFieldClassName
			visible: classid > 0
			enabled: classid > 0

			width: view.width
			anchors.left: view.left

			font: Qaterial.Style.textTheme.headline4
			leadingIconSource: Qaterial.Icons.renameBox
			leadingIconInline: true
			title: qsTr("Az osztály neve")
			backgroundBorderHeight: 1
			backgroundColor: "transparent"
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				QTextFieldInPlaceButtons {
					setTo: classname
					onSaveRequest: {
						Client.send(WebSocket.ApiAdmin, "class/%1/update".arg(classid),
									{
										name: text
									})
						.done(control, function(r){
							Client.reloadCache("classList")
							saved()
						})
						.fail(control, function(err) {
							Client.messageWarning(err, "Átnevezés sikertelen")
							revert()
						})
					}

				}
			}
		}

		Qaterial.TextField {
			id: textFieldCode
			visible: classid != -1
			enabled: classid != -1

			width: view.width
			anchors.left: view.left

			leadingIconSource: Qaterial.Icons.keyChainVariant
			leadingIconInline: true
			title: qsTr("Hitelesítő kód")
			backgroundBorderHeight: 1
			backgroundColor: "transparent"

			validator: RegularExpressionValidator { regularExpression: /.+/ }
			errorText: qsTr("Egyedi kódot szükséges megadni")
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				Qaterial.TextFieldAlertIcon {  }

				Qaterial.TextFieldIconButton
				{
					icon.source: Qaterial.Icons.refresh
					onClicked:
					{
						textFieldCode.text = Client.generateRandomString(6, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
						textFieldCode.textEdited()
					}
				}

				QTextFieldInPlaceButtons {
					setTo: classcode
					onSaveRequest: {
						Client.send(WebSocket.ApiAdmin, "class/%1/updateCode".arg(classid),
									{
										code: text
									})
						.done(control, function(r){
							saved()
						})
						.fail(control, function(err) {
							Client.messageWarning(err, "Módosítás sikertelen")
							revert()
						})
					}

				}


			}

		}

		Qaterial.LabelHeadline5 {
			visible: textFieldClassName.visible
			topPadding: 20
			text: qsTr("Tanulók")

			anchors.left: view.left
		}

		QListView {
			id: view

			currentIndex: -1
			autoSelectChange: true

			width: Math.min(parent.width, Qaterial.Style.maxContainerSize)
			anchors.horizontalCenter: parent.horizontalCenter

			height: contentHeight

			boundsBehavior: Flickable.StopAtBounds
			boundsMovement: Flickable.StopAtBounds

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
			}

			section.property: classid == -1 ? "className" : ""
			section.criteria: ViewSection.FullString
			section.delegate: Qaterial.ListSectionTitle {  }

			delegate: QItemDelegate {
				property User user: model.qtObject
				selectableObject: user

				leftPadding: 0
				rightPadding: 0

				highlighted: ListView.isCurrentItem
				iconSource: user ? (user.roles & _rolePanel) ?
									   Qaterial.Icons.desktopClassic :
									   (user.roles & _roleAdmin) ?
										   Qaterial.Icons.accountCog :
										   (user.roles & _roleTeacher) ?
											   Qaterial.Icons.accountTieHat :
											   Qaterial.Icons.accountSchoolOutline : Qaterial.Icons.accountSchoolOutline

				text: user ? user.fullName: ""
				secondaryText: user ? user.username + (user.oauth.length ? " ("+user.oauth+")" : "") : ""
				textColor: user && user.active ?
							   ((user.roles & _roleAdmin) ? Qaterial.Colors.red300 :
															(user.roles & _roleTeacher) ? Qaterial.Colors.amber300 :
																						  Qaterial.Style.colorTheme.primaryText) :
							   Qaterial.Style.colorTheme.disabledText
				iconColor: user ? (user.selected ?
									   Qaterial.Style.accentColor :
									   user.active ? ((user.roles & _roleAdmin) ?
														  Qaterial.Colors.red300 :
														  (user.roles & _roleTeacher) ?
															  Qaterial.Colors.amber300 :
															  Qaterial.Style.iconColor()) :
													 Qaterial.Style.colorTheme.disabledText)
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
																Client.send(WebSocket.ApiAdmin,
																			model.classid === -1 ? "user/move/none" : "user/move/%1".arg(model.classid),
																			{
																				list: JS.listGetFields(l, "username")
																			})
																.done(control, function(r){
																	reloadUsers()
																	_user.view.unselectAll()
																})
																.fail(control, JS.failMessage("Áthelyezés sikertelen"))
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
	}

	QFabButton {
		visible: view.visible
		action: actionUserAdd
	}

	Action {
		id: actionUserRemove
		icon.source: Qaterial.Icons.accountRemove
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
											.done(control, function(r){
												reloadUsers()
												_user.view.unselectAll()
											})
											.fail(control, JS.failMessage("Törlés sikertelen"))

										},
										title: qsTr("Felhasználók törlése"),
										iconSource: Qaterial.Icons.accountRemove
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
											.done(control, function(r){
												reloadUsers()
												_user.view.unselectAll()
											})
											.fail(control, JS.failMessage("Aktiválás sikertelen"))
										},
										title: qsTr("Felhasználók aktiválása"),
										iconSource: Qaterial.Icons.accountCheck
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
											.done(control, function(r){
												reloadUsers()
												_user.view.unselectAll()
											})
											.fail(control, JS.failMessage("Inaktiválás sikertelen"))
										},
										title: qsTr("Felhasználók inaktiválása"),
										iconSource: Qaterial.Icons.accountOff
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

	function reloadCode() {
		classcode = ""

		if (classid == -1)
			return

		Client.send(WebSocket.ApiAdmin, "class/%1/code".arg(classid < 0 ? -1 : classid))
		.done(control, function(r){
			classcode = r.code
		})
		.fail(control, JS.failMessage("Kód lekérése sikertelen"))
	}

	Component.onCompleted: reloadCode()
	onClassidChanged: reloadCode()
}
