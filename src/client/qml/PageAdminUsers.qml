import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.14
import QtQuick.Dialogs 1.3
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS


Page {
	id: pageAdminUsers

	signal userSelected(var name)
	signal classSelected(var id)

	AdminUsers {
		id: adminUsers
		client: cosClient
	}


	header: QToolBar {
		id: toolbar

		title: qsTr("Felhasználók kezelése")

		backButtonIcon: panelLayout.noDrawer ? CosStyle.iconBack : CosStyle.iconDrawer
		backButton.visible: true
		backButton.onClicked: {
			if (panelLayout.noDrawer)
				mainStack.back()
			else
				panelLayout.drawerToggle()
		}

		Row {
			QToolBusyIndicator { running: adminUsers.isBusy }
			QMenuButton {
				MenuItem {
					text: qsTr("Új pálya")
				}
			}
		}
	}

	Image {
		id: bgImage
		anchors.fill: parent
		fillMode: Image.PreserveAspectCrop
		source: "qrc:/img/villa.png"
	}

	QPanelLayout {
		id: panelLayout
		anchors.fill: parent
	}


	StackView.onRemoved: destroy()

	StackView.onActivated: {
		toolbar.resetTitle()
		panelLayout.panels = [
					{ url: "AdminUserClassList.qml", params: { adminUsers: adminUsers }, fillWidth: false },
					{ url: "AdminUserList.qml", params: { adminUsers: adminUsers }, fillWidth: true },
					{ url: "AdminUserData.qml", params: { adminUsers: adminUsers }, fillWidth: true }
				]
	}

	StackView.onDeactivated: {
		/* UNLOAD */
	}


	function closeDrawer() {
		panelLayout.drawer.close()
	}


	function windowClose() {
		return true
	}


	function stackBack() {
		if (panelLayout.layoutBack()) {
			return true
		}

		if (mainStack.depth > pageAdminUsers.StackView.index+1) {
			if (!mainStack.get(pageAdminUsers.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageAdminUsers.StackView.index+1) {
					mainStack.pop(pageAdminUsers)
				}
			}
			return true
		}

		panelLayout.drawer.close()

		return false
	}
}
