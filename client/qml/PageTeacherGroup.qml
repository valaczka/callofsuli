import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.15
import Qaterial 1.0 as Qaterial
import "./QaterialHelper" as Qaterial
import CallOfSuli 1.0
import "JScript.js" as JS

QPage {
	id: control

	/*stackPopFunction: function() {
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
	}*/

	title: group ? group.name : qsTr("Csoport")
	subtitle: Client.server ? Client.server.serverName : ""

	property StudentGroup group: null

	appBar.backButtonVisible: true
	appBar.rightComponent: Qaterial.AppBarButton
	{
		icon.source: Qaterial.Icons.dotsVertical
		onClicked: swipeView.currentIndex == 0 ? menuClass.open() : menuUser.open()

		QMenu {
			id: menuClass

			QMenuItem { action: actionGroupRename }
			QMenuItem { action: actionGroupRemove }
		}


		QMenu {
			id: menuUser

			QMenuItem { action: actionGroupRename }
		}

	}


	AsyncMessageHandler {
		id: msgHandler

		function groupList(obj : QJsonObject) {
			if (obj.status === "ok")
				Client.setCache("groupListTeacher", obj.list)
			else
				Client.messageWarning(qsTr("Nem sikerült frissíteni az adatokat"))
		}

		function groupModify(obj : QJsonObject) {
			if (obj.status === "ok") {
				Client.snack(qsTr("Csoport módosítva"))
				sendRequestFunc(WebSocketMessage.ClassTeacher, "groupList")
			} else
				Client.messageWarning(qsTr("Nem sikerült létrehozni a csoportot!"))
		}

/*		function userListByClass(obj : QJsonObject) {
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
*/

	}

	Qaterial.SwipeView
	{
		id: swipeView
		anchors.fill: parent
		currentIndex: tabBar.currentIndex

		Rectangle {
			color: "red"
		}

		Rectangle {
			color: "blue"
		}

		Rectangle {
			color: "green"
		}

		Rectangle {
			color: "yellow"
		}
	}

	footer: QTabBar {
		id: tabBar
		currentIndex: swipeView.currentIndex

		Component.onCompleted: {
			model.append({ text: qsTr("Résztvevők"), source: Qaterial.Icons.account, color: "green" })
			model.append({ text: qsTr("Pályák"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Hadjáratok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
			model.append({ text: qsTr("Dolgozatok"), source: Qaterial.Icons.trophyBroken, color: "pink" })
		}
	}

	Action {
		id: actionGroupRemove
		text: qsTr("Törlés")
		enabled: group
		icon.source: Qaterial.Icons.trashCan
		onTriggered: {
			JS.questionDialog(
						{
							onAccepted: function()
							{
								msgHandler.sendRequestFunc(WebSocketMessage.ClassTeacher, "groupRemove", {
															   id: group.groupid
														   })
								Client.stackPop(control.StackView.index-1)
							},
							text: qsTr("Biztosan törlöd a csoportot?"),
							title: group.name,
							iconSource: Qaterial.Icons.closeCircle
						})
		}
	}


	Action {
		id: actionGroupRename
		text: qsTr("Átnevezés")
		enabled: group
		icon.source: Qaterial.Icons.renameBox
		onTriggered: {

			Qaterial.DialogManager.showTextFieldDialog({
														   textTitle: qsTr("Csoport neve"),
														   title: qsTr("Csoport átnevezése"),
														   text: group.name,
														   onAccepted: function(_text, _noerror) {
															   if (_noerror && _text.length)
																   msgHandler.sendRequestFunc(WebSocketMessage.ClassTeacher, "groupModify", {
																								  id: group.groupid,
																								  name: _text
																							  })
														   }
													   })
		}
	}

}