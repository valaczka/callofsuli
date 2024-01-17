import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	title: group ? group.fullName : qsTr("Csoport")
	subtitle: qsTr("Résztvevők szerkesztése")

	property TeacherGroup group: null

	property QTextFieldInPlaceButtons _nameInPlace: null

	stackPopFunction: function() {
		if (_nameInPlace.active) {
			_nameInPlace.revert()
			return false
		}

		if (classView.selectEnabled || userView.selectEnabled) {
			if (classView.selectEnabled) userView.unselectAll()
			if (classView.selectEnabled) classView.unselectAll()
			return false
		}
		return true
	}

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: menu.open()

		QMenu {
			id: menu

			QMenuItem { action: actionClassAdd }
			QMenuItem { action: actionClassRemove }
			Qaterial.MenuSeparator {}
			QMenuItem { action: actionUserAdd }
			QMenuItem { action: actionUserRemove }
		}
	}

	QScrollable
	{
		anchors.fill: parent

		Qaterial.TextField {
			width: parent.width
			font: Qaterial.Style.textTheme.headline4
			leadingIconSource: Qaterial.Icons.accountMultiple
			leadingIconInline: true
			title: qsTr("A csoport neve")
			backgroundBorderHeight: 1
			backgroundColor: "transparent"
			trailingContent: Qaterial.TextFieldButtonContainer
			{
				QTextFieldInPlaceButtons {
					id: nameInPlace
					setTo: group ? group.name: ""
					onSaveRequest: text => {
						Client.send(HttpConnection.ApiTeacher, "group/%1/update".arg(group.groupid),
									{
										name: text
									})
						.done(control, function(r){
							Client.reloadCache("teacherGroupList")
							saved()
						})
						.fail(control, function(err) {
							Client.messageWarning(err, "Átnevezés sikertelen")
							revert()
						})
					}
					Component.onCompleted: control._nameInPlace = nameInPlace
				}
			}

		}

		Qaterial.LabelHeadline5 {
			topPadding: 20
			text: qsTr("Osztályok")
		}

		QListView {
			id: classView

			currentIndex: -1
			width: parent.width
			autoSelectChange: true

			refreshEnabled: false

			model: SortFilterProxyModel {
				sourceModel: group ? group.classList : null

				sorters: [
					StringSorter {
						roleName: "name"
						sortOrder: Qt.AscendingOrder
					}
				]
			}

			delegate: QItemDelegate {
				property ClassObject classobject: model.qtObject
				selectableObject: classobject

				highlighted: ListView.isCurrentItem
				iconSource: Qaterial.Icons.accountMultipleOutline

				text: classobject ? classobject.name : ""

				/*onClicked: if (!view.selectEnabled)
							   Client.stackPushPage("AdminUserEdit.qml", {
														user: user
													})*/
			}

			footer: Column {
				QGreenItemDelegate {
					text: qsTr("Osztály hozzáadása")
					onClicked: actionClassAdd.trigger()
					iconSource: Qaterial.Icons.accountMultiplePlusOutline
					width: classView.width
				}
			}

			Qaterial.Menu {
				id: contextMenu
				QMenuItem { action: classView.actionSelectAll }
				QMenuItem { action: classView.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: actionClassAdd }
				QMenuItem { action: actionClassRemove }
			}

			onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
				if (index != -1)
					currentIndex = index
				contextMenu.popup(mouseX, mouseY)
			}

		}

		Qaterial.LabelHeadline5 {
			text: qsTr("Tanulók")
			topPadding: 20
		}

		QListView {
			id: userView

			currentIndex: -1
			width: parent.width
			autoSelectChange: true

			refreshEnabled: false

			model: SortFilterProxyModel {
				sourceModel: group ? group.userList : null

				sorters: [
					RoleSorter {
						roleName: "classid"
						priority: 1
						sortOrder: Qt.AscendingOrder
					},
					StringSorter {
						roleName: "fullName"
						sortOrder: Qt.AscendingOrder
					}
				]
			}

			delegate: QItemDelegate {
				property User user: model.qtObject
				selectableObject: user

				highlighted: ListView.isCurrentItem
				iconSource: Qaterial.Icons.account

				text: user ? user.fullName: ""
				secondaryText: user ? user.username: ""

				/*onClicked: if (!view.selectEnabled)
							   Client.stackPushPage("AdminUserEdit.qml", {
														user: user
													})*/
			}


			footer: Column {
				QGreenItemDelegate {
					text: qsTr("Tanuló hozzáadása")
					onClicked: actionUserAdd.trigger()
					iconSource: Qaterial.Icons.accountPlus
					width: userView.width
				}
			}

			Qaterial.Menu {
				id: contextMenuUser
				QMenuItem { action: userView.actionSelectAll }
				QMenuItem { action: userView.actionSelectNone }
				Qaterial.MenuSeparator {}
				QMenuItem { action: actionUserAdd }
				QMenuItem { action: actionUserRemove }
			}

			onRightClickOrPressAndHold: (index, mouseX, mouseY) => {
				if (index != -1)
					currentIndex = index
				contextMenuUser.popup(mouseX, mouseY)
			}
		}
	}




	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.httpConnection.pending
	}


	ListModel { id: _dlgClassModel }

	SortFilterProxyModel {
		id: _dlgClassSortModel

		sourceModel: _dlgClassModel

		sorters: [
			StringSorter {
				roleName: "name"
				sortOrder: Qt.AscendingOrder
			}
		]
	}

	ListModel { id: _dlgUserModel }

	SortFilterProxyModel {
		id: _dlgUserSortModel

		sourceModel: _dlgUserModel

		sorters: [
			StringSorter {
				roleName: "familyName"
				sortOrder: Qt.AscendingOrder
				priority: 1
			},
			StringSorter {
				roleName: "givenName"
				sortOrder: Qt.AscendingOrder
			}
		]
	}


	Action {
		id: actionClassAdd
		text: qsTr("Osztály hozzáadása")
		icon.source: Qaterial.Icons.accountMultiplePlusOutline
		enabled: group
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/class/exclude".arg(group.groupid))
			.fail(control, JS.failMessage(qsTr("Letöltsé sikertelen")))
			.done(control, function(r){
				_dlgClassModel.clear()
				for (var i=0; i<r.list.length; i++) {
					var d = r.list[i]
					d.text = d.name
					_dlgClassModel.append(d)
				}

				Qaterial.DialogManager.openCheckListView(
							{
								onAccepted: function(list)
								{
									if (!list.length)
										return

									var clist = []

									for (var j=0; j<list.length; j++)
										clist.push(_dlgClassSortModel.get(list[j]).id)


									Client.send(HttpConnection.ApiTeacher, "group/%1/class/add".arg(group.groupid), { list: clist })
									.fail(control, JS.failMessage(qsTr("Hozzáadás sikertelen")))
									.done(control, function(){group.reload()})

								},
								title: qsTr("Osztály hozzáadása"),
								model: _dlgClassSortModel
							})
			})
		}
	}

	Action {
		id: actionUserAdd
		text: qsTr("Tanuló hozzáadása")
		icon.source: Qaterial.Icons.accountPlus
		enabled: group
		onTriggered: {
			Client.send(HttpConnection.ApiTeacher, "group/%1/user/exclude".arg(group.groupid))
			.fail(control, JS.failMessage(qsTr("Letöltés sikertelen")))
			.done(control, function(r){
				_dlgUserModel.clear()
				for (var i=0; i<r.list.length; i++) {
					var d = r.list[i]
					d.classid = Number(d.classid)
					d.text = d.familyName+" "+d.givenName
					d.secondaryText = d.classname
					_dlgUserModel.append(d)
				}

				Qaterial.DialogManager.openCheckListView(
							{
								onAccepted: function(list)
								{
									if (!list.length)
										return

									var clist = []

									for (var j=0; j<list.length; j++)
										clist.push(_dlgUserSortModel.get(list[j]).username)


									Client.send(HttpConnection.ApiTeacher, "group/%1/user/add".arg(group.groupid), { list: clist })
									.fail(control, JS.failMessage(qsTr("Hozzáadás sikertelen")))
									.done(control, function(){group.reload()})

								},
								title: qsTr("Tanuló hozzáadása"),
								model: _dlgUserSortModel
							})
			})
		}
	}



	Action {
		id: actionClassRemove
		icon.source: Qaterial.Icons.accountMultipleRemoveOutline
		text: qsTr("Osztály eltávolítása")
		onTriggered: {
			var l = classView.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan eltávolítás a kijelölt %1 osztályt?"), "name",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiTeacher,  "group/%1/class/remove".arg(group.groupid), {
															list: JS.listGetFields(l, "classid")
														})
											.done(control, function(r){
												group.reload()
												classView.unselectAll()
											})
											.fail(control, JS.failMessage("Eltávolítás sikertelen"))

										},
										title: qsTr("Osztályok eltávolítása"),
										iconSource: Qaterial.Icons.closeCircle
									})

		}
	}

	Action {
		id: actionUserRemove
		icon.source: Qaterial.Icons.accountRemove
		text: qsTr("Tanuló eltávolítása")
		onTriggered: {
			var l = userView.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan eltávolítás a kijelölt %1 tanulót?"), "fullName",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiTeacher,  "group/%1/user/remove".arg(group.groupid), {
															list: JS.listGetFields(l, "username")
														})
											.done(control, function(r){
												group.reload()
												userView.unselectAll()
											})
											.fail(control, JS.failMessage("Eltávolítás sikertelen"))

										},
										title: qsTr("Tanulók eltávolítása"),
										iconSource: Qaterial.Icons.closeCircle
									})
		}
	}
}
