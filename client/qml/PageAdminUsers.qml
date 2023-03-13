import QtQuick 2.12
import QtQuick.Controls 2.12
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
		}


		QMenu {
			id: menuUser

			QMenuItem { action: actionUserAdd }
		}

	}


	property UserList userList: Client.cache("adminUserList")
	property ClassList classList: Client.cache("classList")

	AsyncMessageHandler {
		id: msgHandler

		signal userPasswordChanged()

		function userListByClass(obj : QJsonObject) {
			if (obj.status === "ok")
				Client.setCache("adminUserList", obj.list)
			else
				Client.messageWarning(qsTr("Nem sikerült frissíteni az adatokat"))
		}

		function classList(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.loadClassListFromArray(obj.list)
			} else
				Client.messageWarning(qsTr("Nem sikerült frissíteni az adatokat!"))
		}

		function classAdd(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Osztály létrehozva"))
				_class.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")
			} else
				Client.messageWarning(qsTr("Nem sikerült létrehozni az osztályt!"))
		}

		function classRemove(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Osztály(ok) törölve"))
				_class.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")
			} else
				Client.messageWarning(qsTr("Nem sikerült törölni az osztályokat!"))
		}

		function classModify(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Osztály módosítva"))
				_class.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")
			} else
				Client.messageWarning(qsTr("Nem sikerült módosítani az osztályt!"))
		}

		function userAdd(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Felhasználó létrehozva"))
				_user.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
				Client.stackPopToPage(control)
			} else
				Client.messageWarning(qsTr("Nem sikerült létrehozni a felhasználót!"))
		}

		function userModify(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Felhasználó módosítva"))
				_user.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
				Client.stackPopToPage(control)
			} else
				Client.messageWarning(qsTr("Nem sikerült módosítani a felhasználót!"))
		}

		function userRemove(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Felhasználó törölve"))
				_user.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
				Client.stackPopToPage(control)
			} else
				Client.messageWarning(qsTr("Nem sikerült törölni a felhasználót!"))
		}

		function userActivate(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Felhasználók módosítva"))
				_user.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
			} else
				Client.messageWarning(qsTr("Nem sikerült módosítani a felhasználókat!"))
		}

		function userMoveToClass(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Felhasználók módosítva"))
				_user.view.unselectAll()
				sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
			} else
				Client.messageWarning(qsTr("Nem sikerült módosítani a felhasználókat!"))
		}

		function userPasswordChange(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Jelszó módosítva"))
				userPasswordChanged()
			} else
				Client.messageWarning(qsTr("Nem sikerült módosítani a jelszót!"))
		}
	}

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
																   msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "classAdd", {
																								  name: _text
																							  })
														   }
													   })
		}
	}


	Action {
		id: actionClassRemove
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.trashCan
		onTriggered: {
			var l = _class.view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 osztályt?"), "name",
									{
										onAccepted: function()
										{
											msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "classRemove", {
																		   list: JS.listGetFields(l, "classid")
																	   })
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
																   msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "classModify", {
																								  classid: o.classid,
																								  name: _text
																							  })
														   }
													   })
		}
	}



	Action {
		id: actionUserAdd
		text: qsTr("Új felhasználó")
		icon.source: Qaterial.Icons.accountPlus
		onTriggered: Client.stackPushPage("AdminUserEdit.qml", {
											  msgHandler: msgHandler,
											  classid: _user.classid
										  })
	}

	StackView.onActivated: {
		msgHandler.sendRequestFunc(WebSocketMessage.ClassAdmin, "userListByClass")
		msgHandler.sendRequestFunc(WebSocketMessage.ClassGeneral, "classList")
	}

}
