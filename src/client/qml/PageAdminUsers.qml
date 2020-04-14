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

		backButtonIcon: panelLayout.noDrawer ? "M\ue5c4" : "M\ue3c7"
		backButton.visible: true
		backButton.onClicked: {
			if (panelLayout.noDrawer)
				mainStack.back()
			else
				panelLayout.drawerToggle()
		}

		rightLoader.sourceComponent: Row {
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
		panelLayout.model.clear()
		panelLayout.model.append(
					{ url: "AdminUserClassList.qml", params: { adminUsers: adminUsers } }
					)
		panelLayout.model.append(
					{ url: "AdminUserList.qml", params: { adminUsers: adminUsers } }
					)
		panelLayout.model.append(
					{ url: "AdminUserData.qml", params: { adminUsers: adminUsers } }
					)
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

		if (mainStack.depth > pageEditor.StackView.index+1) {
			if (!mainStack.get(pageEditor.StackView.index+1).stackBack()) {
				if (mainStack.depth > pageEditor.StackView.index+1) {
					mainStack.pop(pageEditor)
				}
			}
			return true
		}

		panelLayout.drawer.close()

		return false
	}
}
