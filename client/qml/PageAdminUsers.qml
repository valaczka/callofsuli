import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	stackPopFunction: function() {
		if (_class.view.selectEnabled || _user.view.selectEnabled) {
			if (_user.view.selectEnabled) _user.view.unselectAll()
			if (_class.view.selectEnabled) _class.view.unselectAll()
			return false
		}

		if (swipeView.currentIndex > 0) {
			swipeView.setCurrentIndex(0)
			return false
		}

		return true
	}

	title: qsTr("Osztályok és felhasználók")
	subtitle: Client.server ? Client.server.serverName : ""

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: swipeView.currentIndex == 0 ? menuClass.open() : menuUser.open()

		QMenu {
			id: menuClass

			QMenuItem { action: actionClassAdd }
			QMenuItem { action: actionClassRename }
			QMenuItem { action: actionClassRemove }

			Qaterial.MenuSeparator { }

			QMenuItem { action: actionUserImport }
		}


		QMenu {
			id: menuUser

			QMenuItem { action: actionUserAdd }
			QMenuItem { action: actionUserImport }
		}

	}


	UserList { id: userList }

	property ClassList classList: Client.cache("classList")

	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		AdminClassList {
			id: _class
			onClassSelected: {
				_user.view.unselectAll()
				_class.view.unselectAll()
				_user.classid = classid
				_user.classname = classname
				swipeView.setCurrentIndex(1)
			}
		}

		AdminUserList {
			id: _user
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Osztályok"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Felhasználók"), source: Qaterial.Icons.trophyBroken, color: "pink" })
		}
	}


	Action {
		id: actionClassAdd
		text: qsTr("Új osztály")
		icon.source: Qaterial.Icons.plus
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Osztály neve"),
														   title: qsTr("Új osztály létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(WebSocket.ApiAdmin, "class/create", {
																				   name: _text
																			   })
															   .done(function(r){
																   Client.reloadCache("classList")
																   _class.view.unselectAll()
															   })
															   .fail(JS.failMessage("Létrehozás sikertelen"))
														   }
													   })
		}
	}


	Action {
		id: actionClassRemove
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.delete_
		onTriggered: {
			var l = _class.view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 osztályt?"), "name",
									{
										onAccepted: function()
										{
											Client.send(WebSocket.ApiAdmin, "class/delete", {
															list: JS.listGetFields(l, "classid")
														})
											.done(function(r){
												Client.reloadCache("classList")
												_class.view.unselectAll()
											})
											.fail(JS.failMessage("Törlés sikertelen"))
										},
										title: qsTr("Osztályok törlése"),
										iconSource: Qaterial.Icons.closeCircle
									})

		}
	}


	Action {
		id: actionClassRename
		text: qsTr("Átnevezés")
		enabled: _class.view.currentIndex != -1
		icon.source: Qaterial.Icons.renameBox
		onTriggered: {
			var o = _class.view.modelGet(_class.view.currentIndex)
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Osztály neve"),
														   title: qsTr("Osztály átnevezése"),
														   text: o.name,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(WebSocket.ApiAdmin, "class/%1/update".arg(o.classid), {
																				   name: _text
																			   })
															   .done(function(r){
																   Client.reloadCache("classList")
																   _class.view.unselectAll()
															   })
															   .fail(JS.failMessage("Átnevezés sikertelen"))
														   }
													   })
		}
	}



	Action {
		id: actionUserAdd
		text: qsTr("Új felhasználó")
		icon.source: Qaterial.Icons.accountPlus
		onTriggered: Client.stackPushPage("AdminUserEdit.qml", {
											  classid: _user.classid
										  })
	}


	Action {
		id: actionUserImport
		text: qsTr("Importálás")
		icon.source: Qaterial.Icons.import_
		onTriggered: Client.stackPushPage("PageAdminUserImport.qml", {
											 classId: _user.classid
										  })
	}

	function reloadUsers() {
		Client.send(WebSocket.ApiAdmin, "user").done(function(r) {
			Client.callReloadHandler("user", userList, r.list)
		})
		.fail(JS.failMessage("Letöltés sikertelen"))
	}

	StackView.onActivated: {
		_class.view.unselectAll()
		_user.view.unselectAll()
		Client.reloadCache("classList")
		reloadUsers()
	}

}
