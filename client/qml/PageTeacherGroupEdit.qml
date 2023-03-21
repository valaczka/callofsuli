import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import SortFilterProxyModel 0.2
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	title: group ? group.name : qsTr("Csoport")
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
			QMenuItem { action: actionUserAdd }
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
			placeholderText: qsTr("A csoport neve")
			backgroundBorderHeight: 1
			backgroundColor: "transparent"
			trailingContent: QTextFieldInPlaceButtons {
				id: nameInPlace
				setTo: group ? group.name: ""
				onSaveRequest: {
					Client.send(WebSocket.ApiTeacher, "group/%1/update".arg(group.groupid),
								{
									name: text
								})
					.done(function(r){
						Client.reloadCache("teacherGroupList")
						saved()
					})
					.fail(function(err) {
						Client.messageWarning(err, "Átnevezés sikertelen")
						revert()
					})
				}
				Component.onCompleted: control._nameInPlace = nameInPlace
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
				iconSource: Qaterial.Icons.account

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
					iconSource: Qaterial.Icons.plus
					width: classView.width
				}
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
		}
	}


	QRefreshProgressBar {
		anchors.top: parent.top
		visible: Client.webSocket.pending
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
		icon.source: Qaterial.Icons.plus
		enabled: group
		onTriggered: {
			Client.send(WebSocket.ApiTeacher, "group/%1/class/exclude".arg(group.groupid))
			.fail(JS.failMessage(qsTr("Letöltsé sikertelen")))
			.done(function(r){
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
										clist.push(_dlgClassModel.get(list[j]).id)


									Client.send(WebSocket.ApiTeacher, "group/%1/class/add".arg(group.groupid), { list: clist })
									.fail(JS.failMessage(qsTr("Hozzáadás sikertelen")))
									.done(function(){group.reload()})

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
			Client.send(WebSocket.ApiTeacher, "group/%1/user/exclude".arg(group.groupid))
			.fail(JS.failMessage(qsTr("Letöltés sikertelen")))
			.done(function(r){
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
										clist.push(_dlgUserModel.get(list[j]).username)


									Client.send(WebSocket.ApiTeacher, "group/%1/user/add".arg(group.groupid), { list: clist })
									.fail(JS.failMessage(qsTr("Hozzáadás sikertelen")))
									.done(function(){group.reload()})

								},
								title: qsTr("Tanuló hozzáadása"),
								model: _dlgUserSortModel
							})
			})
		}
	}
}
