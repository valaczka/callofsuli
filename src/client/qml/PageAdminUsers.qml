import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


QPage {
	id: pageAdminUsers

	signal userSelected(var name)
	signal classSelected(var id)
	signal createUserRequest()
	signal createClassRequest()

	mainToolBar.title: qsTr("Felhasználók kezelése")

	mainToolBarComponent: QToolBusyIndicator { running: adminUsers.isBusy }

	pageContextMenu: QMenu {
		MenuItem {
			icon.source: CosStyle.iconUserAdd
			text: qsTr("Új felhasználó")
			onClicked: pageAdminUsers.createUserRequest()
		}
		MenuItem {
			icon.source: CosStyle.iconUsersAdd
			text: qsTr("Új osztály")
			onClicked: pageAdminUsers.createClassRequest()
		}
	}

	AdminUsers {
		id: adminUsers
		client: cosClient
	}



	StackView.onActivated: {
		panels = [
					{ url: "AdminUserClassList.qml", params: { adminUsers: adminUsers }, fillWidth: false},
					{ url: "AdminUserList.qml", params: { adminUsers: adminUsers }, fillWidth: true},
					{ url: "AdminUserData.qml", params: { adminUsers: adminUsers }, fillWidth: true}
				]
	}

	onClassSelected: swipeToPage(1)
	onUserSelected: swipeToPage(2)

	function windowClose() {
		return true
	}

	function pageStackBack() {
		return false
	}
}
