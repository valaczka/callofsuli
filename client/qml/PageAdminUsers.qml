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
			QMenuItem { action: actionUserProfileRefresh }
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
			model.append({ text: qsTr("Osztályok"), source: Qaterial.Icons.accountGroupOutline, color: Qaterial.Colors.lightBlue400 })
			model.append({ text: qsTr("Felhasználók"), source: Qaterial.Icons.accountCircle, color: Qaterial.Colors.amber400 })
		}
	}


	Action {
		id: actionClassAdd
		text: qsTr("Új osztály")
		icon.source: Qaterial.Icons.accountMultiplePlusOutline
		onTriggered: {
			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Osztály neve"),
														   title: qsTr("Új osztály létrehozása"),
														   standardButtons: Dialog.Cancel | Dialog.Ok,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   Client.send(HttpConnection.ApiAdmin, "class/create", {
																				   name: _text
																			   })
															   .done(control, function(r){
																   Client.reloadCache("classList")
																   _class.view.unselectAll()
															   })
															   .fail(control, JS.failMessage("Létrehozás sikertelen"))
														   }
													   })
		}
	}


	Action {
		id: actionClassRemove
		text: qsTr("Törlés")
		icon.source: Qaterial.Icons.accountMultipleRemove
		onTriggered: {
			var l = _class.view.getSelected()
			if (!l.length)
				return

			JS.questionDialogPlural(l, qsTr("Biztosan törlöd a kijelölt %1 osztályt?"), "name",
									{
										onAccepted: function()
										{
											Client.send(HttpConnection.ApiAdmin, "class/delete", {
															list: JS.listGetFields(l, "classid")
														})
											.done(control, function(r){
												Client.reloadCache("classList")
												_class.view.unselectAll()
											})
											.fail(control, JS.failMessage("Törlés sikertelen"))
										},
										title: qsTr("Osztályok törlése"),
										iconSource: Qaterial.Icons.accountMultipleRemove
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
																   Client.send(HttpConnection.ApiAdmin, "class/%1/update".arg(o.classid), {
																				   name: _text
																			   })
															   .done(control, function(r){
																   Client.reloadCache("classList")
																   _class.view.unselectAll()
															   })
															   .fail(control, JS.failMessage("Átnevezés sikertelen"))
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
		icon.source: Qaterial.Icons.accountArrowUpOutline
		onTriggered: Client.stackPushPage("PageAdminUserImport.qml", {
											  classId: _user.classid
										  })
	}

	Action {
		id: actionUserProfileRefresh
		text: qsTr("OAuth2 frissítés")
		icon.source: Qaterial.Icons.refresh
		onTriggered: {
			JS.questionDialog({
								  onAccepted: function()
								  {
									  Client.send(HttpConnection.ApiAdmin, "user/update")
									  .done(control, function(r){
										  Client.snack(qsTr("Profilok frissítése..."))
									  })
									  .fail(control, JS.failMessage("Profilok frissítése sikertelen"))
								  },
								  text: qsTr("Frissíted az OAuth2 profilokat?"),
								  title: qsTr("Profilok frissítése"),
								  iconSource: Qaterial.Icons.refresh
							  })
		}


	}

	function reloadUsers() {
		Client.send(HttpConnection.ApiAdmin, "user").done(control, function(r) {
			Client.callReloadHandler("user", userList, r.list)
		})
		.fail(control, JS.failMessage("Letöltés sikertelen"))
	}

	StackView.onActivated: {
		_class.view.unselectAll()
		_user.view.unselectAll()
		Client.reloadCache("classList")
		reloadUsers()
	}

}
