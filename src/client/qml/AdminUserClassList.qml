import QtQuick 2.12
import QtQuick.Controls 2.12
import COS.Client 1.0
import "."
import "Style"
import "JScript.js" as JS

QPagePanel {
	id: panel

	property AdminUsers adminUsers: null

	title: qsTr("Osztályok")

	QPageHeader {
		id: header

		isSelectorMode: classList.selectorSet

		labelCountText: classList.selectedItemCount


		QTextField {
			id: newClassName
			width: parent.width

			validator: RegExpValidator { regExp: /.+/ }

			placeholderText: qsTr("új osztály hozzáadása")
			onAccepted: {
				adminUsers.send({"class": "user", "func": "classCreate", "name": text })
			}
		}


		rightLoader.sourceComponent: QMenuButton {
			id: userMenu
			anchors.verticalCenter: parent.verticalCenter

			MenuItem {
				action: actionRemove
			}
		}

		onSelectAll: classList.selectAll()
	}


	QListItemDelegate {
		id: classList
		anchors.top: header.bottom
		anchors.left: parent.left
		anchors.right: parent.right
		anchors.bottom: parent.bottom

		isObjectModel: true
		modelTitleRole: "name"

		autoSelectorChange: true

		onClicked: pageAdminUsers.classSelected(model.get(index).id)

		onRightClicked: contextMenu.popup()

		QMenu {
			id: contextMenu

			MenuItem { action: actionRemove }
		}
	}

	Action {
		id: actionRemove
		enabled: classList.selectorSet || classList.currentIndex !== -1
		icon.source: CosStyle.iconRemove
		text: qsTr("Törlés")
		onTriggered: deleteClasses()
	}


	Connections {
		target: adminUsers

		onClassListLoaded: getClassList(list)

		onClassCreated: {
			newClassName.clear()
			reloadClassList()
			classList.selectAll(false)
		}

		onClassBatchRemoved:  if (data.error)
								  cosClient.sendMessageWarning(qsTr("Osztályok törlése"), qsTr("Szerver hiba"), data.error)
							  else {
								  reloadClassList()
								  classList.selectAll(false)
							  }
	}

	function populated() {
		if (adminUsers) {
			reloadClassList()
		}
	}

	function reloadClassList() {
		adminUsers.send({"class": "user", "func": "getAllClass"})
	}

	function getClassList(_list) {
		JS.setModel(classList.model, _list)
	}

	function deleteClasses() {
		var l = []
		if (classList.selectorSet)
			l = JS.getSelectedIndices(classList.model, "id")
		else if (classList.currentIndex != -1)
			l.push(classList.model.get(classList.currentIndex).id)

		if (l.length === 0)
			return

		adminUsers.send({"class": "user", "func": "classBatchRemove", "list": l})
	}
}
